#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

uint8_t peerAddress[] = {0x00, 0x4B, 0x12, 0xEE, 0x5E, 0x44};

// ---------------- PIN CONFIG ----------------
#define TOUCH_PIN 4       // Touch sensor (Engine toggle)
#define ENGINE_LED 27
#define BRAKE_LED 33
#define ACC_LED 26

// ---------------- DEVICE CONFIG ----------------
int DEVICE_ID = 2;

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

// ---------------- TOUCH HANDLING ----------------
void checkTouch() {
  static bool touched = false;

  int touchValue = touchRead(TOUCH_PIN);

  // Adjust threshold if needed (~30–50 typical)
  if (touchValue < 40 && !touched) {
    engineOn = !engineOn;
    touched = true;
    delay(300);
  }

  if (touchValue > 50) {
    touched = false;
  }
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

  pinMode(ENGINE_LED, OUTPUT);
  pinMode(BRAKE_LED, OUTPUT);
  pinMode(ACC_LED, OUTPUT);

  // Default AP (no custom IP)
  WiFi.mode(WIFI_AP_STA);
  WiFi.setChannel(1);
  WiFi.softAP("Car_2", "car12345");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // Should be 192.168.4.1

  server.on("/", handleRoot);
  server.on("/acc", handleAcc);
  server.on("/brake", handleBrake);
  server.on("/stop", handleStop);
  server.begin();

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

  Serial.println("Peer added (ESP A)");

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Ready (Car B)");
}

// ---------------- LOOP ----------------
void loop() {

  checkTouch();

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

  // ---------------- DEBUG (IMPROVED) ----------------
  Serial.print("🚗 Car ");
  Serial.print(DEVICE_ID);

  Serial.print(" | Engine: ");
  Serial.print(engineOn ? "ON" : "OFF");

  Serial.print(" | Speed: ");
  Serial.print(speed);

  Serial.print(" | Acc: ");
  Serial.print(acc ? "ON" : "OFF");

  Serial.print(" | Brake: ");
  Serial.print(brake ? "ON" : "OFF");

  Serial.print(" | Touch: ");
  Serial.print(touchRead(TOUCH_PIN));

  Serial.println();
  Serial.println("----------------------------------");

  // ---------------- WEB SERVER ----------------
  server.handleClient();

  delay(1000);
}