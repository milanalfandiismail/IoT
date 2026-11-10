#include <WiFi.h>
#include <HTTPClient.h>

const char* SSID     = "SSID_WIFI";
const char* PASSWORD = "PASSWORD_WIFI";
const char* URL      = "http://10.10.10.25:5000/api/device/state";
const char* TOKEN    = "ganti-device-token";
const int   LED_PIN  = 2;

String authHeader;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  authHeader = String("Bearer ") + TOKEN;
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi tersambung");
}

void loop() {
  static unsigned long lastPoll = 0;
  static bool ledOn = false;
  if (millis() - lastPoll < 1000) return;
  lastPoll = millis();

  HTTPClient http;
  http.setTimeout(2000);
  http.begin(URL);
  http.addHeader("Authorization", authHeader);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    bool on = body.indexOf("true") >= 0;
    if (on != ledOn) {
      ledOn = on;
      Serial.println(on ? "LED ON" : "LED OFF");
    }
    digitalWrite(LED_PIN, on ? HIGH : LOW);
    Serial.printf("GET 200 body: %s", body.c_str());
    if (!body.endsWith("\n")) Serial.println();
  } else {
    Serial.printf("poll gagal: %d\n", code);
  }
  http.end();
}
