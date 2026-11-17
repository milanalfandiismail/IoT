# Naskah Video Presentasi

Dua tugas: **Blynk** dan **Website sendiri**. Durasi total sekitar 7–8 menit.
Teks di bawah dibaca apa adanya, bagian dalam kurung siku adalah arahan kamera.

---

## 1. Pembukaan (± 40 detik)

[Kamera menghadap penyaji, alat di meja sudah menyala]

Selamat pagi, kami dari kelompok … Pada video ini kami mempresentasikan dua tugas IoT yang sudah kami kerjakan. Tugas pertama adalah sistem pemantauan suhu dan kontrol lampu menggunakan **platform Blynk**. Tugas kedua adalah sistem yang sama, tetapi kami bangun sendiri dari nol: **server web, database, dan dashboard** milik kami sendiri.

Kedua alat memakai komponen yang sama, yaitu ESP32 sebagai pengendali, satu LED sebagai output, dan sensor DHT sebagai input suhu. Yang membedakan hanya ke mana datanya dikirim.

---

## 2. Tugas 1 — Blynk

### 2.1 Alat dan wiring (± 60 detik)

[Kamera close-up ke board]

Perangkat yang kami pakai ada empat. ESP32 sebagai mikrokontroler, sensor DHT11 sebagai pembaca suhu, LED sebagai indikator, dan kabel jumper.

Wiring-nya sederhana. Sensor DHT11 punya tiga kaki yang dipakai: VCC disambungkan ke pin 3.3 volt, GND ke ground, dan pin data ke **GPIO4**, atau pin D4 pada board. LED kami sambungkan ke **GPIO2**, yang pada board ESP32 sudah tersedia sebagai lampu bawaan, jadi tidak perlu resistor tambahan. Kalau memakai LED eksternal, tetap perlu resistor 220 sampai 330 ohm supaya arusnya aman.

[Nyalakan alat, tunjukkan LED menyala]

### 2.2 Block diagram (± 50 detik)

[Tampilkan `block-diagram-blynk-1.png`]

Block diagram-nya terdiri dari empat kotak. ESP32 berada di sisi perangkat, terhubung ke router atau hotspot lewat WiFi. Router meneruskan koneksi ke internet, lalu masuk ke Blynk Cloud. Di sisi pengguna, aplikasi Blynk di ponsel juga terhubung ke cloud yang sama.

Poin pentingnya: board **selalu membuka koneksi keluar**. Tidak ada port yang dibuka di ESP32, dan ponsel tidak perlu berada di jaringan yang sama dengan board. Selama keduanya punya internet, keduanya bisa bertemu di Blynk Cloud.

### 2.3 Flowchart (± 60 detik)

[Tampilkan `flowchart-blynk-1.png`]

Alurnya dimulai saat board dinyalakan. ESP32 menyambungkan diri ke WiFi, lalu menyambung ke Blynk menggunakan auth token. Setelah tersambung, ada dua jalur yang berjalan bersamaan.

Jalur pertama adalah pengiriman data. Setiap lima detik board membaca sensor suhu, mengirim nilainya ke virtual pin V3, lalu aplikasi menampilkannya dalam bentuk angka dan grafik.

Jalur kedua adalah kontrol. Saat tombol di aplikasi digeser, Blynk mengirim perintah ke board. Board menerima nilai pada virtual pin V2, lalu menyalakan atau mematikan GPIO2 sesuai nilainya.

Satu hal yang kami tambahkan: setiap kali board tersambung ulang, board meminta status terakhir tombol ke server. Jadi kalau listrik sempat mati, posisi tombol di aplikasi tetap sesuai kondisi lampu yang sebenarnya.

### 2.4 Demo (± 40 detik)

[Demo: geser tombol V2, LED merespons; tunjukkan suhu di aplikasi]

Ini hasilnya. Tombol di aplikasi saya geser, lampu langsung mengikuti. Angka suhu di aplikasi ikut berubah setiap beberapa detik sesuai pembacaan sensor.

---

## 3. Tugas 2 — Website Sendiri

### 3.1 Gambaran umum (± 40 detik)

[Tampilkan `block-diagram-1.png` atau `block-diagram-lan.png`]

Untuk tugas kedua, kami tidak memakai platform pihak ketiga. Kami membuat server sendiri memakai **Flask**, framework Python, yang berjalan di server Proxmox di jaringan laboratorium, dengan alamat IP `10.10.10.3` port `5000`. Datanya disimpan di **SQLite**, satu file database berisi dua tabel: tabel perangkat dan tabel pengguna.

Jadi ada tiga bagian: perangkat ESP32 di lapangan, server yang kami kelola sendiri, dan dashboard yang dibuka lewat browser.

### 3.2 Wiring (± 30 detik)

[Close-up board]

Wiring-nya sama seperti tugas pertama: LED di GPIO2, tambahan LED kedua di GPIO4, dan sensor DHT22 data di GPIO4. Bedanya hanya tipe sensornya, DHT22 yang lebih akurat.

### 3.3 Block diagram (± 50 detik)

[Tampilkan `block-diagram-lan.png`]

Pada versi ini, ESP32 adalah pengirim permintaan, server yang menjawab. Board bertanya setiap satu detik ke alamat `api/device/state`, membawa token perangkat di header Authorization. Server mencocokkan token itu ke database, lalu membalas apakah lampu seharusnya menyala atau mati.

Untuk sensor suhu arahnya terbalik: board yang mengirim data ke `api/device/temperature`, server menyimpannya, lalu dashboard menampilkannya.

Kami juga menyiapkan dua jalur akses. Pertama lewat jaringan lokal, langsung ke IP server. Kedua lewat domain dan Cloudflare Tunnel, jadi alat bisa dikontrol dari luar jaringan laboratorium.

### 3.4 Flowchart (± 70 detik)

[Tampilkan `flowchart-ringkas.png`]

Alurnya begini. Pengguna membuka dashboard dan login. Setelah masuk, pengguna menekan tombol pada kartu perangkat. Server menyimpan perubahan itu ke database, dan memperbarui waktu terakhir perangkat diperbarui.

Di sisi perangkat, ESP32 terus bertanya. Setiap permintaan, server memeriksa dua hal: apakah token perangkat terdaftar dan aktif, dan apakah perangkat itu memang bertipe LED. Kalau token salah, server membalas 401 dan tidak terjadi apa-apa. Kalau benar, board menerima jawaban berisi status lampu, lalu menyalakan atau mematikan pin sesuai status itu.

Untuk suhu, alurnya kebalikan: sensor dibaca, dikirim ke server, divalidasi rentangnya antara minus lima puluh sampai delapan puluh derajat, baru disimpan.

### 3.5 Keamanan dan akun (± 50 detik)

[ Tampilkan halaman akun ]

Yang membedakan versi ini dari Blynk adalah sisi pengelolaannya. Setiap perangkat punya token sendiri, dan token itu bisa dicabut kapan saja dari dashboard. Kalau token dicabut, perangkat langsung ditolak dan tidak bisa lagi mengubah apa pun.

Untuk pengguna, setiap anggota kelompok punya akun sendiri. Password disimpan dalam bentuk hash, bukan teks asli. Ada dua peran: admin yang bisa menambah dan menghapus anggota, dan anggota biasa yang hanya bisa mengganti password sendiri. Semua anggota melihat data perangkat yang sama.

### 3.6 Demo (± 60 detik)

[Demonstrasi: login, kartu perangkat, klik tombol, LED menyala, suhu tampil]

Saya login sebagai admin. Di dashboard ada kartu perangkat: dua LED dan satu sensor suhu. Saya tekan tombol pada kartu LED pertama, lampu di board menyala dalam sekitar satu detik. Saya tekan lagi, lampu mati.

Kartu suhu menampilkan angka terakhir dari sensor beserta keterangan berapa detik lalu data itu masuk. Kalau board dimatikan, angka terakhir tetap ada karena tersimpan di database, dan keterangannya akan terus bertambah.

Di halaman akun, admin bisa menambah anggota baru, mereset password, atau menghapus akun. Di sini juga ada daftar token perangkat yang bisa dicabut.

---

## 4. Perbandingan Singkat (± 40 detik)

[Tampilkan dua alat berdampingan]

Kalau dibandingkan, versi Blynk unggul di kecepatan pengerjaan. Tidak ada server yang harus diurus, tidak ada database, dan aplikasinya sudah jadi. Kekurangannya, kami bergantung pada layanan pihak ketiga dan tidak bisa mengatur sendiri aturan aksesnya.

Versi website butuh waktu jauh lebih lama karena harus membuat server, database, autentikasi, dan tampilan. Tapi hasilnya sepenuhnya milik kami: token perangkat bisa dicabut sendiri, akun anggota bisa dikelola sendiri, dan data tersimpan di server kami.

---

## 5. Penutup (± 20 detik)

Dari dua tugas ini kami belajar bahwa inti sistem IoT ada di tiga hal: perangkat yang membaca atau mengubah kondisi fisik, jalur komunikasi yang membawa datanya, dan antarmuka tempat manusia mengendalikannya. Platform siap pakai mempercepat pekerjaan, sedangkan membangun sendiri memberi kendali penuh.

Sekian presentasi dari kami, terima kasih.

---

## Catatan teknis untuk perekaman

- Rekam demo Blynk dan demo website **terpisah**, jangan gabung satu take.
- Saat demo website, pastikan board sudah menyala dan token di sketch cocok dengan yang ada di dashboard.
- Kalau memakai jaringan kampus, pakai alamat IP lokal `http://10.10.10.3:5000` supaya tidak bergantung pada internet.
- Kalau memakai domain, tunggu sampai log board menunjukkan `bo
