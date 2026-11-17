# Block Diagram Versi Blynk

Tanpa server Flask. Board bicara langsung ke Blynk Cloud.

```mermaid
flowchart LR
    subgraph LOKAL["Jaringan lokal"]
        direction TB
        ESP["ESP32<br/>LED GPIO2<br/>DHT11 data GPIO4"]
        RT["Router / hotspot WiFi<br/>SSID TM_2.4G<br/>bypass MAC di MikroTik"]
    end
    subgraph CLOUD["Cloud"]
        direction TB
        NET["Internet"]
        BL["Blynk Cloud<br/>Template: nama template Blynk<br/>V2 = LED, V3 = suhu"]
        APP["Aplikasi Blynk di HP"]
    end
    ESP <-->|"TCP keluar"| RT
    RT --> NET
    NET <--> BL
    BL <-->|"tombol V2, grafik V3"| APP
```

## Keterangan

- ESP32 sebagai **client**, membuka koneksi keluar ke Blynk Cloud. Tidak ada port yang dibuka di board.
- Aplikasi Blynk di HP juga membuka koneksi ke cloud yang sama, jadi keduanya tidak perlu satu jaringan.
- `V2` menulis ke board (tombol ON/OFF), `V3` dibaca dari board (suhu tiap 5 detik).
- Tidak ada database: status LED dan suhu terakhir disimpan di server Blynk.
