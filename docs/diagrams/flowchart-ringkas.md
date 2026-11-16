# Flowchart Ringkas (untuk presentasi)

Satu gambar, alur inti sistem. Versi rinci tetap ada di `flowchart.md`.

```mermaid
flowchart TB
    U["Dashboard<br/>klik ON / OFF"] --> S["Flask<br/>cek token + simpan"]
    D["Sensor DHT22"] --> S
    S --> R["ESP32 polling<br/>tiap 1 detik"]
    S --> V["Dashboard<br/>tampilkan suhu"]
    R --> G["GPIO<br/>LED nyala / mati"]
    G --> R
    S --> X["401 bila token salah"]
    S --> B[("SQLite<br/>devices + users")]
```

## Kalimat pengantar saat presentasi

- Pengguna menekan tombol di dashboard, Flask menyimpannya ke SQLite.
- ESP32 tidak pernah "disuruh" server. Board bertanya sendiri tiap 1 detik dengan bearer token.
- Token dicek ke database; kalau tidak cocok, server membalas 401 dan tidak ada aksi.
- Untuk suhu arahnya terbalik: DHT22 membaca, ESP32 yang mengirim ke server tiap 5 detik, dashboard menampilkan angka terakhir.

Gambar: `flowchart-ringkas.png` (1600 x 1046, latar putih).
