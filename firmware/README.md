# Firmware — Rev A

The sketch lives in [`plant_monitor/`](plant_monitor/). It is written for the
Arduino IDE, because that is what I code in, and it is split across a handful of
files in one sketch folder — the IDE opens them as tabs, so it is still one
thing to open and one button to press.

**Status: written, builds clean, not yet run on hardware.** The boards were ordered
on 2026-08-21 and arrive early September. Everything below is written against
the hardware contract in the table further down, not against measurements from
the real board. The soil calibration constants in particular are breadboard
values, and bring-up step D-11 is where they get replaced with numbers taken
from the PCB. Until then the percentage is directionally right and absolutely
suspect, and I would not read anything into a single figure.

It builds for the ESP32-S3 against Arduino-ESP32 2.0.17 with Adafruit BME280
2.3.0, Adafruit VEML7700 2.1.6 and PubSubClient 2.8.0, with no warnings —
750 kB of flash and 48 kB of RAM, about a fifth of the app partition. That is
all a clean build proves. It is not a test, and not one line of this has met the
actual board yet.

---

## The hardware contract

This is what the *board* requires of the firmware. It was written down before
any code existed, so the safety and measurement behaviour would be on record
either way. Pin assignments are in
[`../docs/PinMap_CheatSheet.md`](../docs/PinMap_CheatSheet.md).

| Duty | Rule | Why, and where it comes from |
|---|---|---|
| **Wi-Fi TX cutoff** | No transmit below **≈3.5 V** battery | A regulator rule, not a battery rule: the AP2112K needs ≈200 mV of headroom to hold 3.3 V through a 355 mA TX peak — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §4 |
| **Deep-sleep cutoff** | Shut everything down at **3.0 V** | The pack's protection module does not trip until 2.5 V. That half-volt gap belongs to firmware and nothing else — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §3 |
| **Battery reporting** | Gate battery percentage on **USB absent** | With USB connected and no cell fitted, the charger floats VBAT to ≈4.2 V and BAT_SENSE reads "full" |
| **Soil measurement** | GPIO21 LOW → wait ≥200 ms → read GPIO1 → GPIO21 HIGH | The probe is firmware-switched so it does not drain the cell between readings |
| **Soil calibration** | Dry ≈ **2900** raw, wet ≈ **1465** raw (12-bit, ATTEN=3) | Breadboard values through the as-built input chain. Re-record on the PCB at bring-up step D-11 before trusting absolute readings — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §1 |
| **I²C** | BME280 at **0x76**, VEML7700 at **0x10**; `Wire.begin(38, 39)` — SDA first | Bus re-pinned to IO38/IO39 during layout — [`PinMap_CheatSheet.md`](../docs/PinMap_CheatSheet.md) |
| **Brown-out** | Expect a ≈50–100 ms dip on VSYS at USB unplug | Discrete load-sharing hand-over. If a reset appears there at low battery, relax the brown-out detector across the transition — [`BringUp_Guide.md`](../docs/BringUp_Guide.md) step 9 |

Every one of those rows is enforced somewhere in the code, and the code says so
at the point it does it.

---

## What one wake cycle does

There is no `loop()` in the usual sense. The board wakes from deep sleep, runs
`setup()` once, and sleeps again at the end of it.

1. **Read the battery first**, before anything expensive has loaded the cell.
2. **Below 3.0 V, stop.** No readings, no radio — sleep 6 hours and check again.
   I keep waking, rarely, because a board that stopped waking entirely could
   never notice it had been put on the charger.
3. **Switch the soil probe on** and start its 250 ms settle clock.
4. **Read the BME280 and VEML7700 while that clock runs.** The settle time has
   to be spent either way, so it may as well be spent working.
5. **Read the probe, switch it off.** Releasing GPIO21 rather than driving it
   high lets R10 hold Q1 off with no drive current, through sleep and through
   the next boot.
6. **Below 3.5 V, skip the radio** and keep the readings to myself this cycle.
   Otherwise associate, publish one retained JSON message, disconnect.
7. **Sleep 30 minutes.**

A hard 30-second deadline sits over the whole thing. Any stage that finds it
gone gives up rather than starting something new — losing one reading is much
cheaper than sitting at 80 mA on a cell I want to last a hundred days.

---

## The files

| File | What is in it |
|---|---|
| [`plant_monitor.ino`](plant_monitor/plant_monitor.ino) | The wake cycle, in order, and nothing else |
| [`config.h`](plant_monitor/config.h) | Every number the board imposes — pins, thresholds, calibration, timing — with the document each came from |
| `secrets.h` | Wi-Fi and broker credentials. **Gitignored.** Never committed |
| [`secrets.h.example`](plant_monitor/secrets.h.example) | The template to copy |
| [`power.h`](plant_monitor/power.h) / [`.cpp`](plant_monitor/power.cpp) | Battery reading, the two thresholds, the discharge curve, and the only route to deep sleep |
| [`sensors.h`](plant_monitor/sensors.h) / [`.cpp`](plant_monitor/sensors.cpp) | The three measurements, each allowed to fail without costing the cycle |
| [`net.h`](plant_monitor/net.h) / [`.cpp`](plant_monitor/net.cpp) | Wi-Fi, MQTT, and the Home Assistant discovery payloads |
| [`adc.h`](plant_monitor/adc.h) | The one place either analogue input gets read |
| [`log.h`](plant_monitor/log.h) | Printing that compiles away entirely when it is switched off |

---

## Building it

### 1. Board support

Arduino IDE → **File → Preferences → Additional board manager URLs**, add:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Then **Tools → Board → Boards Manager**, install **esp32 by Espressif Systems**.

### 2. Libraries

**Tools → Manage Libraries**, install:

* **Adafruit BME280 Library** (pulls in Adafruit Unified Sensor and Adafruit BusIO)
* **Adafruit VEML7700 Library**
* **PubSubClient** by Nick O'Leary

### 3. Credentials

Copy `plant_monitor/secrets.h.example` to `plant_monitor/secrets.h` and fill it
in. The IDE will not show `.example` as a tab, so do this in a file manager or a
terminal, then reopen the sketch.

```
copy plant_monitor\secrets.h.example plant_monitor\secrets.h     :: Windows
cp   plant_monitor/secrets.h.example plant_monitor/secrets.h     #  macOS/Linux
```

`secrets.h` is in [`.gitignore`](../.gitignore) and stays there. If it is
missing the build stops with a message saying so rather than failing somewhere
confusing.

### 4. Board settings

**Tools → Board → esp32 → ESP32S3 Dev Module**, then:

| Setting | Value | Why |
|---|---|---|
| USB CDC On Boot | **Enabled** | `Serial` is the native USB. There is no bridge chip on this board |
| USB Mode | **Hardware CDC and JTAG** | IO19/IO20 go straight to the USB Serial/JTAG peripheral |
| Upload Mode | **UART0 / Hardware CDC** | |
| Flash Size | **8MB (64Mb)** | The module is a WROOM-1-**N8** |
| PSRAM | **Disabled** | N8 has none. N8**R2** would |
| Partition Scheme | **8M with spiffs (3MB APP/1.5MB SPIFFS)** | |
| CPU Frequency | 240 MHz | Finishing the wake cycle sooner beats running it slower |

If it will not enter the bootloader on its own: hold **SW1 (BOOT)**, tap
**SW2 (RESET)**, release SW1, then upload.

---

## Things I know are not right yet

* **There is no VBUS sense line on Rev A.** I never brought +5V_PROT to a GPIO,
  so the firmware cannot actually tell whether USB is supplying the board. What
  it can tell is whether a USB *host* has opened the CDC port, and it uses that
  as a stand-in for gating the battery percentage. That is right at the bench
  and wrong on a phone charger, which supplies VBUS and never opens a port. A
  divider from +5V_PROT to a spare pin fixes it properly — it is on the Rev B
  list.
* **The calibration constants are from a breadboard.** Bring-up step D-11
  replaces them.
* **`BAT_TRIM` is 1.0**, meaning I have not yet compared the firmware's battery
  reading against a meter on C16. Step C-8.
* **No OTA.** Every update is over the USB cable. On a board that spends 99.9 %
  of its life in deep sleep, an OTA window costs more current than it saves me
  walking over to it.
* **Nothing is written to flash between wakes.** If I later want gap-free
  history through a Wi-Fi outage, the readings would have to go somewhere that
  survives sleep. Right now a missed publish is simply a missed reading.
