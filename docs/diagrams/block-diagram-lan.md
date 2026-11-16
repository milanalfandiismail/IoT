# Block Diagram Versi LAN

Tanpa domain dan tanpa Cloudflare Tunnel. Semua perangkat ada di jaringan lokal yang sama.

```mermaid
flowchart LR
    subgraph LAP["Perangkat ESP32 (satu jaringan)"]
        direction TB
        L1["ESP32 LED 1<br/>GPIO2"]
        L2["ESP32 LED 2<br/>GPIO4"]
        D1["ESP32 DHT22<br/>GPIO4"]
    end
    RT["Router / hotspot<br/>LAN 10.10.10.0/24"]
    subgraph SRV["Server Proxmox 10.10.10.3:5000"]
        direction TB
        APP["Flask app.py<br/>session + bearer token"]
        DB[("SQLite data.db<br/>devices / users")]
        APP <--> DB
    end
    U["Browser di jaringan yang sama<br/>dashboard + halaman akun"]
    L1 --> RT
    L2 --> RT
    D1 --> RT
    RT <-->|"HTTP polling / POST suhu"| APP
    U <-->|"HTTP langsung tanpa domain"| APP
```

## Bedanya dengan versi domain

| | Versi domain (`block-diagram-1.png`) | Versi LAN (ini) |
|---|---|---|
| Alamat | `http://10.10.10.3:5000` | `http://10.10.10.3:5000` |
| Lewat internet | ya, Cloudflare Tunnel | tidak |
| Butuh internet | ya | tidak |
| Latensi | kena jalur Cloudflare | langsung di LAN |
| Sertifikat TLS (versi https) | perlu CA di sketch | tidak perlu |

Catatan teknis: di LAN, `WiFi.config` dengan DNS `9.9.9.9` tetap boleh dipakai, tapi tidak lagi berpengaruh karena host tujuannya IP, bukan nama domain.
