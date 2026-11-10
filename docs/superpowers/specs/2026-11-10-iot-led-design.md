# Desain: Kontrol LED ESP32 via Flask (HTTP Polling)

**Tanggal:** 2026-11-10
**Status:** Disetujui (permintaan "gas" dari pemilik proyek)

## Tujuan

Tugas kuliah/sekolah: menyalakan dan mematikan satu LED di ESP32 dari jarak jauh.
Server Flask menjadi pusat kontrol, ESP32 menarik perintahnya sendiri lewat HTTP polling.

## Yang dibangun

- **Server Flask**: API dengan bearer token, halaman login, dashboard tombol ON/OFF.
- **Firmware ESP32** (Arduino C++): polling `GET /api/device/state` tiap 1 detik, menyalakan GPIO2.
- **Tes otomatis** untuk seluruh perilaku server (`pytest`).

## Non-goals (sengaja tidak dibuat)

- MQTT, WebSocket, atau push notification.
- Multi-device, multi-LED, multi-user.
- Database. State LED disimpan di memori proses; restart server = LED kembali ke `off`.
- HTTPS/TLS, OTA firmware, scheduling, histori.
- Firmware untuk dikompilasi otomatis di CI (hanya dikompilasi manual).

## Pendekatan yang dipilih

| Pendekatan | Keputusan |
|---|---|
| Transport | **HTTP polling 1 detik.** MQTT ditolak: butuh broker terpisah, dan kehilangan pesan saat device offline butuh kode tambahan. |
| Auth API | **Token statis** dari `.env`: `API_TOKEN` (manusia/curl) dan `DEVICE_TOKEN` (ESP32). Dipisah agar token device tidak bisa dipakai membuka dashboard. |
| Auth web | **Session cookie** Flask + satu akun admin dari `.env` (password di-hash `werkzeug`). Login JWT lengkap ditolak: tidak ada multi-user. |
| Penyimpanan state | Dict di memori: `STATE = {"on": False}`. |
| Firmware | Arduino C++, `HTTPClient`, penjadwalan `millis()`, `setTimeout(2000)`. |

## Kontrak perilaku

### Endpoint

| Method | Path | Auth | Body / Hasil |
|---|---|---|---|
| `GET` | `/login` | publik | Form login |
| `POST` | `/login` | publik | Form `username`, `password`. Benar → 302 ke `/` + session. Salah → 401 + pesan error |
| `POST` | `/logout` | session | Hapus session, 302 ke `/login` |
| `GET` | `/` | session | Dashboard: status LED + tombol ON/OFF. Tanpa session → 302 ke `/login` |
| `POST` | `/api/led` | session **atau** `Bearer API_TOKEN` | `{"on": true\|false}` → 200 `{"on": bool}`. Body tidak valid → 400. Token salah/tidak ada → 401 |
| `GET` | `/api/device/state` | **hanya** `Bearer DEVICE_TOKEN` | 200 `{"on": bool}` |

### Aturan validasi

- Body wajib JSON object dengan key `on` bertipe **boolean**. `1`, `"true"`, `null` → 400.
- Scheme `Authorization` tidak peka huruf besar/kecil (`bearer` dan `Bearer` diterima).
- Header `Authorization` hilang → 401, bukan 500.
- `DEVICE_TOKEN` tidak boleh berlaku di `/api/led`, `API_TOKEN` tidak boleh berlaku di `/api/device/state`.

### Konfigurasi (`.env`)

```
SECRET_KEY=<string acak>
ADMIN_USER=admin
ADMIN_PASSWORD_HASH=<hasil generate_password_hash>
API_TOKEN=<token manusia>
DEVICE_TOKEN=<token esp32>
```

## Alur

```
ESP32 ──GET /api/device/state (Bearer DEVICE_TOKEN)──► Flask   tiap 1 detik
      ◄── {"on": true} ───────────────────────────────
                                     ▲
Browser ──POST /login → session──┐   │
        ──POST /api/led {"on":true}──┘
```

## Risiko yang diketahui

- LED baru berubah maksimal 1 detik setelah perintah (keterlambatan polling).
- State hilang saat server restart; ESP32 akan menyamakan diri dalam 1 detik.
- HTTP tanpa TLS: token bisa disadap di jaringan yang sama. Dapat diterima untuk tugas di LAN pribadi.
