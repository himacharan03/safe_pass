#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

uint8_t peerAddress[] = {0xF0, 0x24, 0xF9, 0x5A, 0xD0, 0x04};

// ---------------- PIN CONFIG ----------------
#define BUTTON_PIN 12
#define ENGINE_LED 27
#define BRAKE_LED 33
#define ACC_LED 26

// ---------------- DEVICE CONFIG ----------------
int DEVICE_ID = 1;   // CHANGE: 1, 2, 3 for different ESPs

// ---------------- VARIABLES ----------------
bool engineOn = false;
bool brake = false;
bool acc = false;
float speed = 0;

// ---------------- WEB SERVER ----------------
WebServer server(80);

// ---------------- STRUCT ----------------
typedef struct struct_message {
  int id;
  float speed;
  int acc;
  int brake;
  int engine;
} struct_message;

struct_message myData;
struct_message incomingData;

// ---------------- BUTTON HANDLING ----------------
void checkButton() {
  static bool lastState = HIGH;
  bool current = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && current == LOW) {
    engineOn = !engineOn;
    delay(300); // debounce
  }
  lastState = current;
}

// ---------------- WEB HANDLERS ----------------
void handleRoot() {
  String html = "<h2>Car " + String(DEVICE_ID) + " Control</h2>";

  html += "<p>Engine: " + String(engineOn ? "ON" : "OFF") + "</p>";
  html += "<p>Speed: " + String(speed) + "</p>";

  html += "<a href='/acc'>Accelerate</a><br>";
  html += "<a href='/brake'>Brake</a><br>";
  html += "<a href='/stop'>Stop</a><br>";

  server.send(200, "text/html", html);
}

void handleAcc() {
  if (engineOn) {
    acc = true;
    brake = false;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBrake() {
  if (engineOn) {
    brake = true;
    acc = false;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop() {
  brake = false;
  acc = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------------- ESP-NOW RECEIVE ----------------
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));

  Serial.print("📡 Received from Car ");
  Serial.print(incomingData.id);
  Serial.print(" | Speed: ");
  Serial.print(incomingData.speed);
  Serial.print(" | Engine: ");
  Serial.print(incomingData.engine);
  Serial.print(" | Acc: ");
  Serial.print(incomingData.acc);
  Serial.print(" | Brake: ");
  Serial.println(incomingData.brake);
}

// ---------------- ESP-NOW SEND ----------------
void sendData() {
  myData.id = DEVICE_ID;
  myData.engine = engineOn;
  myData.acc = acc;
  myData.brake = brake;
  myData.speed = speed;

  esp_now_send(peerAddress, (uint8_t *) &myData, sizeof(myData));
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENGINE_LED, OUTPUT);
  pinMode(BRAKE_LED, OUTPUT);
  pinMode(ACC_LED, OUTPUT);

  // ---------------- WIFI SETUP ----------------
  WiFi.mode(WIFI_AP_STA);

  IPAddress local_ip(192, 168, 10, DEVICE_ID); // unique IP per car
  IPAddress gateway(192, 168, 10, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.setChannel(1);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  String ssid = "Car_" + String(DEVICE_ID);
  WiFi.softAP(ssid.c_str(), "car010234");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // ---------------- WEB SERVER ----------------
  server.on("/", handleRoot);
  server.on("/acc", handleAcc);
  server.on("/brake", handleBrake);
  server.on("/stop", handleStop);
  server.begin();

  // ---------------- ESP-NOW ----------------
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));

  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Peer added (ESP B)");

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Ready (V2V Enabled)");
}

// ---------------- LOOP ----------------
void loop() {

  checkButton();

  // ---------------- SPEED LOGIC ----------------
  if (engineOn) {
    if (acc) speed += 2;
    if (brake) speed -= 3;
  } else {
    speed = 0;
  }

  speed = constrain(speed, 0, 100);

  // ---------------- LED OUTPUT ----------------
  digitalWrite(ENGINE_LED, engineOn);
  digitalWrite(BRAKE_LED, brake);
  digitalWrite(ACC_LED, acc);

  // ---------------- SEND DATA ----------------
  sendData();

  // ---------------- DEBUG ----------------
  Serial.print("🚗 Car ");
  Serial.print(DEVICE_ID);
  Serial.print(" | Speed: ");
  Serial.print(speed);
  Serial.print(" | Engine: ");
  Serial.print(engineOn);
  Serial.print(" | Acc: ");
  Serial.print(acc);
  Serial.print(" | Brake: ");
  Serial.println(brake);

  // ---------------- HANDLE WEB ----------------
  server.handleClient();

  delay(1000);
}