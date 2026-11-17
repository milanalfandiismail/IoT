# Block Diagram Versi Blynk

```mermaid
flowchart LR
    ESP["ESP32<br/>LED + sensor suhu"] <-->|"WiFi"| RT["Router / hotspot"]
    RT <-->|"Internet"| BL["Blynk Cloud"]
    BL <--> APP["Aplikasi Blynk di HP"]
```

Gambar: `block-diagram-blynk-1.png`.
