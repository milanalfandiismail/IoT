# Flowchart Sistem IoT LED & Suhu

Empat alur, masing-masing satu gambar siap tempel.

## 1. Koneksi dan autentikasi (ESP32)

```mermaid
flowchart TB
    A([Boot ESP32]) --> B[Sambung WiFi, DNS 9.9.9.9]
    B --> C{Dapat IP?}
    C -- tidak --> B
    C -- ya --> D["GET /api/device/state<br/>Bearer token"]
    D --> E{Token ada di devices<br/>dan enabled = 1?}
    E -- tidak --> F[401 unauthorized]
    E -- ya --> G[Lanjut ke tahap 2]
```

## 2. Aksi LED di board

```mermaid
flowchart TB
    A[Response dari server] --> B{type = led?}
    B -- tidak --> C[400 bukan device LED]
    B -- ya --> D["JSON on: true/false"]
    D --> E{Beda dengan GPIO?}
    E -- ya --> F[digitalWrite LED ON/OFF]
    E -- tidak --> G[Tidak berubah]
    F --> H[Tunggu 1 detik]
    G --> H
    C --> H
    H --> A
```

## 3. Alur pengguna di dashboard

```mermaid
flowchart TB
    A([Buka domain]) --> B{Ada session?}
    B -- tidak --> C["GET /login"]
    C --> D{Password cocok?}
    D -- tidak --> E[401 salah] --> C
    D -- ya --> F[session user_id] --> G[Dashboard]
    B -- ya --> G
    G --> H["fetch /api/devices tiap 2 detik"]
    H --> I{Tombol LED diklik?}
    I -- ya --> J["POST /api/devices/id/led"]
    J --> K[UPDATE devices SET led_on] --> H
    I -- tidak --> H
```

## 4. Kirim suhu (ESP32 + DHT22)

```mermaid
flowchart TB
    A([Boot ESP32]) --> B[Sambung WiFi]
    B --> C["Baca DHT22 tiap 5 detik"]
    C --> D{Pembacaan valid?}
    D -- tidak --> C
    D -- ya --> E["POST /api/device/temperature"]
    E --> F{Token cocok dan<br/>type = suhu?}
    F -- tidak --> G[401 / 400] --> C
    F -- ya --> H{celsius -50..80?}
    H -- tidak --> I[400] --> C
    H -- ya --> J[UPDATE temperature] --> C
```

## File gambar

| File | Isi |
|---|---|
| `flowchart-1.png` | koneksi + autentikasi ESP32 |
| `flowchart-2.png` | aksi LED di board |
| `flowchart-3.png` | alur pengguna dashboard |
| `flowchart-4.png` | kirim suhu DHT22 |
