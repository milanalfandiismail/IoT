/*
   Sketch uji: hanya WiFi, tanpa Blynk dan tanpa DHT.
   Tujuan: memisahkan masalah hardware/daya dari library.
   Kalau sketch ini ikut crash -> bukan kode Blynk.
*/

#include <WiFi.h>

char ssid[] = "ISI_SSID";
char pass[] = "ISI_PASSWORD";

unsigned long terakhir = 0;
unsigned int putaran = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("[Uji] mulai");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  unsigned long mulai = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - mulai < 15000) delay(500);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.printf("[Uji] MAC %02X:%02X:%02X:%02X:%02X:%02X IP %s\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                WiFi.localIP().toString().c_str());
}

void loop() {
  if (millis() - terakhir < 2000) return;
  terakhir = millis();
  putaran++;

  Serial.printf("[Uji] putaran %u status %d IP %s RSSI %d\n",
                putaran, WiFi.status(),
                WiFi.localIP().toString().c_str(), WiFi.RSSI());

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
  }
}
