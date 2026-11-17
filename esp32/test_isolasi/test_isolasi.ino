/*
   Sketch uji isolasi: nyalakan satu per satu untuk mencari sumber panic.

   1) PAKAI_BLYNK 1, PAKAI_DHT 0  -> uji WiFi + Blynk saja
   2) PAKAI_BLYNK 0, PAKAI_DHT 1  -> uji WiFi + DHT saja
   3) dua-duanya 0                -> uji WiFi saja (paling ringan)

   Kirim log 60 detik untuk tiap kombinasi.
*/

#define PAKAI_BLYNK 1
#define PAKAI_DHT   0

#define BLYNK_TEMPLATE_ID   "ISI_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ISI_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "ISI_TOKEN_BLYNK"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#if PAKAI_DHT
#include <DHT.h>
#endif

char ssid[] = "ISI_SSID";
char pass[] = "";

#if PAKAI_DHT
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#endif

BlynkTimer timer;
unsigned int putaran = 0;

BLYNK_WRITE(V2) {
  Serial.printf("[Uji] V2 = %d\n", param.asInt());
}

void berkala() {
  putaran++;
  Serial.printf("[Uji] putaran %u, WiFi status %d, IP %s\n",
                putaran, WiFi.status(), WiFi.localIP().toString().c_str());
#if PAKAI_DHT
  float t = dht.readTemperature();
  Serial.printf("[Uji] DHT %s %.1f C\n", isnan(t) ? "gagal" : "ok", t);
#endif
}

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

#if PAKAI_DHT
  dht.begin();
#endif
#if PAKAI_BLYNK
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);
#endif
  timer.setInterval(3000L, berkala);
}

void loop() {
#if PAKAI_BLYNK
  Blynk.run();
#endif
  timer.run();
}
