# Pin Map Cheat Sheet — ESP32-S3 Plant Monitor Rev A
*Supersedes `references/ESP32S3_PlantMonitor_RevA_PinMap_CheatSheet.pdf` (which still showed the deleted soil divider). Designators match the schematic. Updated 2026-07-20.*

## At a glance

* **I²C:** SCL = GPIO4 · SDA = GPIO5 · pull-ups R8/R9 = 4.7 k to +3V3 (one pair for the whole bus). Net names: `SCL`, `SDA`.
* **Sensors:** BME280 = 0x76 (SDO→GND, CSB→3V3) · VEML7700 = 0x10 (fixed).
* **ADC1 (ATTEN=3, effective 0–2900 mV):** GPIO1 = `ADC_SOIL` (ADC1_CH0, **direct** — no divider; 100 k pull-down R11 + 100 nF C14) · GPIO2 = `BAT_SENSE` (ADC1_CH1, VBAT÷2 via R14/R15 100 k 1 % + 100 nF C17).
* **Soil-probe power:** GPIO21 = `SENS_PWR_EN` → Q1 gate. **LOW = probe ON**, HIGH/floating = off (R10 100 k pull-up keeps it off in deep sleep and at boot).
* **USB (native):** GPIO19 = D− (`USB_DN`) · GPIO20 = D+ (`USB_DP`). No bridge chip.
* **Buttons (per schematic refs — note: design doc has these swapped):** **SW1 = BOOT** (GPIO0, hold low to flash) · **SW2 = RESET** (EN).
* **Strapping pins:** GPIO0 (10 k pull-up R6), GPIO3 / GPIO45 / GPIO46 left unconnected on purpose.
* **Soil calibration (direct chain, 12-bit raw):** wet ≈ 1465 · dry ≈ 2900.

## Module pins used (U3, ESP32-S3-WROOM-1-N8)

| Pin | Name | Net | Role |
|---|---|---|---|
| 2 | 3V3 | +3V3 | Power in (C9 22 µF + C10 100 nF at pin) |
| 3 | EN | — | Reset: R7 10 k up, C8 1 µF, SW2 |
| 27 | IO0 | — | Boot strap: R6 10 k up, SW1 |
| 13 | IO19 | USB_DN | Native USB D− (22 Ω R1 in line) |
| 14 | IO20 | USB_DP | Native USB D+ (22 Ω R2 in line) |
| 4 | IO4 | SCL | I²C clock |
| 5 | IO5 | SDA | I²C data |
| 39 | IO1 | ADC_SOIL | Soil reading, ADC1_CH0, direct |
| 38 | IO2 | BAT_SENSE | VBAT÷2, ADC1_CH1 |
| 23 | IO21 | SENS_PWR_EN | Probe power switch, LOW = on |
| 37/36 | TXD0/RXD0 | — | Unused (no pads on Rev A — add TPs at layout) |
| 1, 40, 41 | GND/EPAD | GND | Ground + thermal |

## Net glossary (as built)

| Net | Meaning |
|---|---|
| USB_VBUS | Raw 5 V at the connector, before the fuse |
| +5V_PROT | 5 V after F1 — feeds charger and D4; also Q3's gate reference (R16 10 k pull-down) |
| VSYS | System node: ~4.5–4.65 V on USB, ≈ VBAT on battery. Feeds the LDO |
| +3V3 | Regulated rail (AP2112K, U2) |
| VBAT_RAW / VBAT | Battery + before / after reverse-protection Q2 |
| BAT_SENSE | VBAT ÷ 2 → GPIO2 (2.10 V max) |
| SOIL_PWR | Switched 3.3 V to probe (J2 pin 2, middle) |
| ADC_SOIL | Probe output → GPIO1 (J2 pin 1); 100 k to GND |
| SCL / SDA | Sensor bus |
| USB_DP / USB_DN | USB data to GPIO20/19 |

## Connectors

* **J2 (soil, JST-PH 3):** 1 = ADC_SOIL · **2 = SOIL_PWR (middle)** · 3 = GND — matches DFRobot Gravity order (signal/VCC/GND). Buzz out the cable anyway.
* **J3 (battery, JST-PH 2):** 1 = VBAT_RAW (+) · 2 = GND. **Meter the pack plug before first connection.**

## Probe points (no TPs on Rev A)

+5V_PROT → C2/F1.2 · VSYS → C5/C18 · +3V3 → C7/C9 · VBAT → C16 · BAT_SENSE → C17 · ADC_SOIL → C14/R11 · EN → C8 · IO0 → R6/SW1 · GND → USB shell / J3.2
