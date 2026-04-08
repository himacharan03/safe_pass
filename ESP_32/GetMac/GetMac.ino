#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin("dummy", "dummy");   // 🔥 FORCE WIFI INIT
  delay(500);

  Serial.println("Getting MAC Address...");

  String mac = WiFi.macAddress();

  Serial.print("ESP MAC Address: ");
  Serial.println(mac);
}

void loop() {}