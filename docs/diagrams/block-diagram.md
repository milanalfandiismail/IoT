# Block Diagram Sistem IoT LED & Suhu

Komponen dan jalur komunikasi yang dipakai aplikasi saat ini.

```mermaid
flowchart LR
    subgraph LAP["Perangkat lapangan"]
        direction TB
        L1["ESP32 #1<br/>LED, GPIO2<br/>poll tiap 1 detik"]
        L2["ESP32 #2<br/>LED, GPIO4<br/>poll tiap 1 detik"]
        D1["ESP32 #3<br/>DHT22, GPIO4<br/>kirim tiap 5 detik"]
    end

    subgraph NET["Jaringan"]
        direction TB
        RT["Router / hotspot WiFi"]
        CF["Domain publik<br/>Cloudflare Tunnel"]
    end

    subgraph SRV["Server Proxmox 10.10.10.3:5000"]
        direction TB
        APP["Flask app.py<br/>session login + bearer token"]
        DB[("SQLite data.db<br/>devices / users")]
        APP <--> DB
    end

    U["Browser HP / laptop<br/>dashboard + halaman akun"]

    L1 -->|"HTTP GET /api/device/state"| RT
    L2 -->|"HTTP GET /api/device/state"| RT
    D1 -->|"HTTP POST /api/device/temperature"| RT
    RT --> CF
    CF <-->|"HTTPS"| APP
    U <-->|"HTTPS: login, tombol LED"| CF
```

## Keterangan

- ESP32 adalah **client**, server tidak pernah menghubungi board. Board yang bertanya tiap 1 detik.
- Satu token = satu device. Token dikirim sebagai header `Authorization: Bearer <token>`.
- LED: server membalas `{"on": true|false}`, board yang menyalakan GPIO.
- Suhu: arah data terbalik, board mengirim `{"celsius": 26.5}`.
- `data.db` menyimpan token device (hash + plaintext) dan akun dashboard.

![Block diagram](block-diagram-1.png)
