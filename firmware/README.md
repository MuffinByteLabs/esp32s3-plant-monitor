ESP32-S3 firmware lives here (PlatformIO or ESP-IDF project).
Key duties from the hardware side: no Wi-Fi TX below ~3.5 V battery, deep-sleep
everything at 3.0 V, gate battery-% on USB-absent, soil probe: GPIO21 LOW, wait
200 ms, read GPIO1, GPIO21 HIGH.
