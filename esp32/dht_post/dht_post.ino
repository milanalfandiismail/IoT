#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* SSID     = "SSID_WIFI";
const char* PASSWORD = "PASSWORD_WIFI";
const char* URL      = "http://10.10.10.25:5000/api/device/temperature";
const char* TOKEN    = "ganti-device-token";
const int   DHT_PIN  = 4;
const int   DHT_TYPE = DHT22;  // DHT11 -> ganti ke DHT11

DHT dht(DHT_PIN, DHT_TYPE);
String authHeader;

void setup() {
  Serial.begin(115200);
  dht.begin();
  authHeader = String("Bearer ") + TOKEN;
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi tersambung");
}

void loop() {
  static unsigned long lastRead = 0;
  // DHT11 tidak boleh dibaca lebih sering dari ~1x per detik
  if (millis() - lastRead < 5000) return;
  lastRead = millis();

  float celsius = dht.readTemperature();
  if (isnan(celsius)) {
    Serial.println("DHT gagal dibaca");
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus, data dibuang");
    return;
  }

  HTTPClient http;
  http.setTimeout(2000);
  http.begin(URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", authHeader);
  String body = String("{\"celsius\":") + String(celsius, 1) + "}";
  int code = http.POST(body);
  Serial.printf("POST %d body: %s\n", code, body.c_str());
  http.end();
}
