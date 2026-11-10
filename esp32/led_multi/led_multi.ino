#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* SSID     = "SSID_WIFI";
const char* PASSWORD = "PASSWORD_WIFI";

// Skema URL menentukan mode: https:// pakai TLS terverifikasi,
// http:// (mis. http://10.10.10.25:5000) tetap jalan untuk tes LAN.
const char* URL = "http://10.10.10.25:5000/api/device/state";

// Root CA yang dipercaya bila URL memakai https://. Default: GlobalSign ECC
// Root CA - R4 (rantai Google Trust Services) dan ISRG Root X1 (Let's Encrypt).
// Ganti dengan root CA server-mu sendiri kalau memakai sertifikat lain.
static const char CA_CERT[] = R"CERT(
-----BEGIN CERTIFICATE-----
MIIB3DCCAYOgAwIBAgINAgPlfvU/k/2lCSGypjAKBggqhkjOPQQDAjBQMSQwIgYD
VQQLExtHbG9iYWxTaWduIEVDQyBSb290IENBIC0gUjQxEzARBgNVBAoTCkdsb2Jh
bFNpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMTIxMTEzMDAwMDAwWhcNMzgw
MTE5MDMxNDA3WjBQMSQwIgYDVQQLExtHbG9iYWxTaWduIEVDQyBSb290IENBIC0g
UjQxEzARBgNVBAoTCkdsb2JhbFNpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wWTAT
BgcqhkjOPQIBBggqhkjOPQMBBwNCAAS4xnnTj2wlDp8uORkcA6SumuU5BwkWymOx
uYb4ilfBV85C+nOh92VC/x7BALJucw7/xyHlGKSq2XE/qNS5zowdo0IwQDAOBgNV
HQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUVLB7rUW44kB/
+wpu+74zyTyjhNUwCgYIKoZIzj0EAwIDRwAwRAIgIk90crlgr/HmnKAWBVBfw147
bmF0774BxL4YSFlhgjICICadVGNA3jdgUM/I2O2dgq43mLyjj0xMqTQrbO/7lZsm
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

struct Lampu {
  const char* nama;   // label di Serial Monitor
  const char* token;  // token device dari kartu dashboard
  uint8_t pin;
  bool nyala;
  unsigned long terakhir;
};

// Satu baris = satu LED di dashboard. Tambah LED dengan menambah baris.
// Pin aman: 2, 4, 5, 12-19, 21-23, 25-27, 32, 33. Hindari 6-11 (flash),
// 34-39 (input only), dan 0/2/12/15 saat boot bisa mengganggu upload.
Lampu lampu[] = {
  {"led-1", "ganti-token-led-1", 2, false, 0},
  {"led-2", "ganti-token-led-2", 4, false, 0},
};
const int JUMLAH = sizeof(lampu) / sizeof(lampu[0]);

WiFiClientSecure tls;
WiFiClient biasa;

// Host dari URL, untuk uji DNS/TCP sebelum polling.
String hostUrl(const char* url) {
  String s(url);
  int a = s.indexOf("://");
  int b = s.indexOf('/', a + 3);
  return s.substring(a + 3, b < 0 ? s.length() : b);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < JUMLAH; i++) {
    pinMode(lampu[i].pin, OUTPUT);
    digitalWrite(lampu[i].pin, LOW);
  }
  tls.setCACert(CA_CERT);
  Serial.print("URL: ");
  Serial.println(URL);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi tersambung");
  // Verifikasi sertifikat butuh jam yang benar; tanpa NTP jam ESP32 masih 1970.
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  time_t sekarang = time(nullptr);
  int tunggu = 0;
  Serial.print("sinkron waktu");
  while (sekarang < 1700000000 && tunggu < 40) {   // maks 20 detik
    delay(500);
    Serial.print(".");
    sekarang = time(nullptr);
    tunggu++;
  }
  Serial.println();
  if (sekarang < 1700000000) Serial.println("NTP gagal, HTTPS akan ditolak");
  else Serial.printf("waktu: %s", ctime(&sekarang));

  // Uji jalur: DNS dulu, lalu TCP port 443. Kalau salah satu gagal,
  // masalahnya jaringan, bukan sertifikat/token.
  String host = hostUrl(URL);
  IPAddress ip;
  if (WiFi.hostByName(host.c_str(), ip)) Serial.printf("DNS %s -> %s\n", host.c_str(), ip.toString().c_str());
  else Serial.printf("DNS %s gagal\n", host.c_str());
  WiFiClient uji;
  // Uji beberapa port: kalau 443 ditutup jaringan, Cloudflare masih bisa
  // diakses lewat 8443/2053 (port HTTPS yang diproksikan Cloudflare).
  const int portUji[] = {80, 443, 8443, 2053};
  for (int p : portUji) {
    if (uji.connect(host.c_str(), p)) {
      Serial.printf("TCP %d OK\n", p);
      uji.stop();
    } else {
      Serial.printf("TCP %d gagal\n", p);
    }
  }
}

void poll(int i) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi putus");
    WiFi.reconnect();
    return;
  }

  HTTPClient http;
  http.setTimeout(5000);
  bool https = String(URL).startsWith("https://");
  if (!(https ? http.begin(tls, URL) : http.begin(biasa, URL))) {
    Serial.printf("%s: begin gagal\n", lampu[i].nama);
    return;
  }
  http.addHeader("Authorization", String("Bearer ") + lampu[i].token);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    bool on = body.indexOf("true") >= 0;
    if (on != lampu[i].nyala) {
      lampu[i].nyala = on;
      Serial.printf("%s (GPIO%d) %s\n", lampu[i].nama, lampu[i].pin, on ? "ON" : "OFF");
    }
    digitalWrite(lampu[i].pin, on ? HIGH : LOW);
    Serial.printf("%s body: %s\n", lampu[i].nama, body.c_str());
  } else {
    Serial.printf("%s: poll gagal: %d (%s)\n", lampu[i].nama, code,
                  HTTPClient::errorToString(code).c_str());
    if (https) {
      char pesan[128];
      int err = tls.lastError(pesan, sizeof(pesan));
      Serial.printf("  TLS error: %d (%s), WiFi %d dBm\n", err, pesan, WiFi.RSSI());
    }
  }
  http.end();
}

void loop() {
  for (int i = 0; i < JUMLAH; i++) {
    if (millis() - lampu[i].terakhir < 1000) continue;
    lampu[i].terakhir = millis();
    poll(i);
  }
  delay(10);
}
