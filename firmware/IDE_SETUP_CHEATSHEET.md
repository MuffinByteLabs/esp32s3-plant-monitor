# Arduino IDE Setup — ESP32-S3 Plant Monitor Rev A

*Bench card. Started 2026-09-09 after losing time to a fresh IDE session with default settings.*
*The long-form version lives in [`README.md`](README.md) §Building it — this is the part I actually
need in front of me.*

---

## The settings, in one table

Select the board **first**. The Tools menu only grows these options once a board is chosen, and it
fills them with generic defaults that are wrong for this hardware in three places.

| Tools → | Default | **Set to** | Why |
|---|---|---|---|
| Board | — | **ESP32S3 Dev Module** | esp32 by Espressif Systems |
| Port | — | **the Espressif one** (see below) | not the Bluetooth ones |
| **USB CDC On Boot** | Disabled | **Enabled** ⚠️ | **The one that wastes afternoons.** Left disabled, `Serial` goes to the UART0 pins instead of USB. Board runs, uploads, and prints nothing. Looks dead. Is not |
| **Flash Size** | 4MB | **8MB (64Mb)** | the module is a WROOM-1-**N8** |
| **Partition Scheme** | Default 4MB | **8M with spiffs (3MB APP/1.5MB SPIFFS)** | must match the flash size above |
| PSRAM | Disabled | **Disabled** ✓ | N8 has none. N8**R2** would |
| USB Mode | Hardware CDC and JTAG | **Hardware CDC and JTAG** ✓ | IO19/IO20 go straight to the USB Serial/JTAG peripheral |
| Upload Mode | UART0 / Hardware CDC | **UART0 / Hardware CDC** ✓ | |
| CPU Frequency | 240 MHz | **240 MHz** ✓ | finishing a wake cycle sooner beats running it slower |

## Choosing board and port

**IDE 2.x** — the dropdown in the toolbar, next to ✓ and →:
`dropdown → Select other board and port… → type "ESP32S3 Dev Module" → pick the port → OK`

**IDE 1.x** — `Tools → Board → esp32 → ESP32S3 Dev Module`, then `Tools → Port`.

## Finding the right COM port

The dropdown lists Bluetooth serial ports too, and they look just as plausible. Identify the board by
its **USB vendor ID: `VID_303A` is Espressif**. Paste into PowerShell:

```powershell
Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match 'COM\d+' } | Select-Object Name, DeviceID
```

The board is the row whose DeviceID contains `VID_303A`. It shows up as a plain
**"USB Serial Device (COMx)"** — Windows has no idea what it is, because this board has no bridge chip
and uses the ESP32-S3's own USB peripheral.

*Was COM5 on 2026-09-09. It can move — re-run the command rather than trusting this line.*

## Libraries

`Tools → Manage Libraries`:

| Library | Needed by |
|---|---|
| Adafruit BME280 Library | all sketches (pulls in Adafruit Unified Sensor + BusIO) |
| Adafruit VEML7700 Library | all sketches |
| **PubSubClient** (Nick O'Leary) | `plant_monitor` — **required even with `USE_MQTT 0`**, because `net.cpp` includes the header unconditionally |

## The sketches

| Sketch | Sleeps? | For |
|---|---|---|
| `sensor_check` | no | quick sensor read. Single ADC sample on soil — do not calibrate from it |
| `bringup_check` | no | fuller diagnostic, 16-sample averaging, reset reason at boot only |
| `handover_check` | no | bring-up steps 9 & 10. Reset reason **and uptime on every line**, mirrored to UART0 |
| `plant_monitor` | **yes** | the real firmware. Deep sleeps between wakes |

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Uploads fine, serial monitor silent | **USB CDC On Boot = Disabled** | set it to Enabled |
| No COM port in the dropdown | not enumerating | hold **SW1 (BOOT)**, tap **SW2 (RESET)**, release SW1 |
| Port appears and vanishes every second | blank flash — ROM finds no app, resets, retries | hold SW1 while plugging USB in, hold 2 s more, release. Flash once and it never recurs |
| **Port vanishes after ~10 s, comes back later** | `plant_monitor` deep-sleeping. **Not a fault** | to re-flash: hold SW1, tap SW2, release, upload |
| Edits to a file seem to do nothing | **the IDE does not reload files changed on disk** | close the tab and reopen the sketch |
| `secrets.h` tab missing | it does not exist, or the sketch was opened before it did | create it from `secrets.h.example`, then reopen the sketch |
| `PubSubClient.h: No such file` | library not installed | see Libraries above |
| A `#if SOMETHING` block silently vanishes | the macro is used **above** the `#include` that defines it — the preprocessor reads an undefined name as `0` and warns about nothing | move the `#if` below the include, or drop the guard |
| `curl: A positional parameter cannot be found` | PowerShell aliases `curl` to `Invoke-WebRequest` | use **`curl.exe`** — any curl command off the internet needs the `.exe` in PowerShell |

## Board-specific facts worth not relearning

* **There is no GPIO-controlled LED.** D2 is hardwired to +3V3 through R5; D3 belongs to the charger.
  "Is it alive?" is answered over serial, never by blinking something.
* **Silk labels are net names, not TP numbers**: `5V VSYS 3V3 VBAT BAT_SNS SIG GND EN IO0 TX RX`.
* **I²C is IO38/IO39** — `Wire.begin(38, 39)`, SDA first. The old IO4/IO5 numbers appear in retired
  docs and will silently find nothing.
* **SW1 = BOOT, SW2 = RESET.**
* **Recovery UART**: adapter TXD→TP12, adapter RXD→TP11, GND→TP8, 115200.
  **3.3 V logic only — never connect the adapter's 5 V or 3V3 power pin.**
