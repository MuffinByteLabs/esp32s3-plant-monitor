# Bring-Up Guide — ESP32-S3 Plant Monitor Rev A
*Supersedes `references/ESP32S3_PlantMonitor_RevA_BringUp_Checklist.pdf` (2026-07-20). Designators match the schematic, not the design doc.*

**Bench kit:** multimeter, current-limited USB source if available (set ~700 mA), oscilloscope (needed for step 9), the buzzed-out probe cable, a metered battery.

---

## 0. Rules before anything touches the board

1. **Battery polarity ritual (non-negotiable).** Meter the pack's plug before it ever meets J3: pin 1 = positive, pin 2 = GND. LiPo pigtails have no color standard, and PKCell packs have shipped with "reversed" plugs relative to JST convention.
2. **The charge LED does NOT prove polarity.** With USB plugged in and a *reversed* battery, the charger's precondition current turns the reverse-protection FET on and trickles ~10 mA backwards into the cell — while D3 glows a steady, normal-looking "charging." Reverse protection is only complete with USB absent. Meter first, always.
3. **Protected cells only** (pack must contain a PCM). The board has no protection IC of its own.
4. **Never store the board with a flat battery plugged in.** Sleep floor is ~210 µA; a 500 mAh cell hits the PCM's 2.5 V cutoff in ~100 days, and the PKCell spec lists 0 V-recovery charging as *Unavailable* — a fully dead cell is scrap.
5. Charge only between 0–45 °C (pack spec).

## 1. Quirks to keep in mind (what's "normal" for this board)

* **USB-unplug hand-over (the R16 story).** Q3 (battery switch) is held OFF by its gate sitting on the +5V_PROT rail, which carries ~14.7 µF of capacitance. When USB is pulled, that stored charge keeps Q3 off while the board is fed through Q3's *body diode* at a 0.65 V penalty — with the original 100 k pull-down this lasted 0.5–1 s, dropping the 3.3 V rail to ~2.9–3.1 V on a mid-charge battery and tripping the ESP32-S3's ~2.98 V brown-out reset. **R16 is now 10 k (changed 2026-07-20)**, shrinking the window to ~50–100 ms, which the VSYS capacitors and the brown-out margin ride through. Cost: 0.5 mA extra draw from USB only (5 V ÷ 10 k), zero on battery. *Step 9 verifies this on real hardware.*
* **Power LED (D2) is deliberately dim.** 10 k series resistor ⇒ ~120 µA. Faint in daylight is expected. It is also the single largest sleep-mode drain — unsolder it for battery-life measurements.
* **Charge LED (D3) flickers faintly with no battery connected.** Charger STAT pin tri-states; cosmetic, expected.
* **BAT_SENSE lies when USB is present with no battery:** the charger floats VBAT to ~4.2 V, so the ADC reads "full battery." Firmware should gate battery-% reporting on USB absence.
* **LDO gets warm on USB during sustained Wi-Fi** (~0.26 W). Fine for bursts; watch it during long OTA sessions (thermal shutdown at ~160 °C protects it).
* **Firmware must enforce two battery thresholds** (the hardware only *reports*):
  * **~3.5 V — stop transmitting.** Regulator rule: the AP2112K needs ~0.2 V headroom to hold 3.3 V during a 355 mA TX peak. Sensors + sleep still fine below this.
  * **3.0 V — deep sleep, stop everything.** Battery rule (PKCell discharge floor). The pack's own PCM doesn't trip until 2.5 V — that half-volt gap is firmware's responsibility.

## 2. Probe points (TP1–TP12 fitted 2026-07-22; backup pads for tight spots)

| Net | TP | Backup pad | Net | TP | Backup pad |
|---|---|---|---|---|---|
| +5V_PROT | TP1 | C2 (+) / F1 pin 2 | BAT_SENSE | TP5 | C17 (+) |
| VSYS | TP2 | C5 / C18 (+) | ADC_SOIL | TP6 | C14 (+) / R11 top |
| +3V3 | TP3 | C7 / C9 (+) | EN | TP9 | C8 / R7 junction |
| VBAT | TP4 | C16 (+) | IO0 | TP10 | R6 / SW1 junction |
| GND | TP7, TP8\* | USB shell / J3 pin 2 | TXD0 / RXD0 | TP11\*, TP12\* | — (module pins 37/36) |
| USB_VBUS | — | C1 (+) | | | |

\* Through-hole 1.0 mm pads — a header pin or DuPont jumper end fits and holds. **Recovery UART / early-boot logs:** adapter TXD→TP12 (RXD0), adapter RXD→TP11 (TXD0), GND→TP8; 115200 baud; to flash, hold BOOT (SW1) and tap RESET (SW2). 3.3 V logic only — never connect an adapter's 5 V / 3V3 power pins.

## 3. Sequence

**A — no power**
1. Visual: orientation of U1–U6, D1–D4, Q1–Q3; no bridges; polarity marks.
2. Meter resistance GND↔ each of +5V_PROT / VSYS / +3V3 / VBAT: no shorts (all ≥ kΩ-range and climbing as caps charge).

**B — USB only (no battery)**

3. Plug USB. Expected: **USB_VBUS ≈ 5.0 V · +5V_PROT ≈ 4.87–4.96 V · VSYS ≈ 4.45–4.65 V · +3V3 = 3.30 V**. Power LED faintly on. Input current at idle: tens of mA.
4. Board enumerates as a USB serial device; flash a hello/blink build. (Native USB — no bridge chip. If it won't enumerate: try another cable, check D+/D− aren't swapped at U1, re-check the J1 GND-pin fix.)
5. I²C scan finds **0x76** (BME280) and **0x10** (VEML7700).
6. Read all sensors; sanity-check values.

**C — battery**

7. **Meter the pack plug** (rule 0.1), then connect at J3 with USB still in. Charge LED on; VBAT (C16) climbs toward 4.20 V; charge current ≈ 90–110 mA (your as-received cell measured 3.95 V, so expect a shortish CC phase then taper).
8. BAT_SENSE = VBAT ÷ 2 (±1 %). Firmware reading ×2 should match the meter at C16.
9. **Hand-over test (scope).** Battery at ~3.7–3.9 V for a worthwhile test. Scope on VSYS (TP2; C18 (+) works too), trigger single, ~50 ms/div. Pull USB.
   * **PASS:** VSYS steps down and rides at ≈ VBAT − 0.65 V for **≤ ~100 ms**, then snaps up to ≈ VBAT and stays; board keeps running, no reset, +3V3 never drops below ~3.0 V.
   * **FAIL (old behavior):** the notch lasts ~0.5–1 s and the board reboots → R16 isn't the 10 k part, or the gate node has extra capacitance.
10. Battery-only cold boot: unplug USB, press reset (SW2) — board must boot and run from battery alone.

**D — sensors, sleep, soak**

11. Buzz out the probe cable, plug into J2. Power the probe from firmware (GPIO21 LOW), wait ≥ 200 ms, read. Record **dry (in air)** and **in water** raw values; breadboard reference (direct, no divider, 12 dB attenuation): **wet ≈ 1465, dry ≈ 2900 counts (~1.1 V / ~2.2 V)**. On-board values should land close; re-record them as the board's calibration constants.
12. Sleep current in series with the battery: expect **~210 µA** as built (D2 120 µA + LDO 55 µA + divider 21 µA + ESP32 ~10 µA), or **~90 µA with D2 removed**.
13. Wi-Fi range sanity check, then a 24 h soak: charge behavior, readings every wake, no resets (check reset-reason register in logs).

## 4. Troubleshooting quick table

| Symptom | First suspects |
|---|---|
| No enumeration | Cable (try several), J1 GND pins, D+/D− continuity through R1/R2, drivers |
| Resets when USB unplugged | R16 value (must be 10 k), battery too low, scope the VSYS notch |
| Charge LED never lights | Battery polarity (meter it!), R12/D3 orientation, VBAT wiring |
| Charge never terminates | Load on VBAT during charge shouldn't exist on this design — check Q3 isolation (Vgs at USB-present should be ≈ +0.4 V) |
| ADC values noisy | Average 8–16 samples; keep probe leads short; confirm C14/C17 fitted |
| BME280 missing at 0x76 | SDO must be at GND; check CSB at 3V3 |
| LDO very hot | Sustained TX on USB is the worst case (~0.26 W) — airflow/copper; brief is fine |

## 5. Record sheet

| Item | Expected | Measured | Pass |
|---|---|---|---|
| +5V_PROT (USB) | 4.87–4.96 V | | |
| VSYS (USB) | 4.45–4.65 V | | |
| +3V3 | 3.30 V ± 2 % | | |
| Charge current | 90–110 mA | | |
| VBAT at termination | 4.168–4.232 V | | |
| BAT_SENSE ÷ VBAT | 0.50 ± 1 % | | |
| Hand-over notch | ≤ ~100 ms, no reset | | |
| Soil dry / wet raw | ≈ 2900 / ≈ 1465 | | |
| Sleep current | ~210 µA (~90 µA no LED) | | |
| I²C scan | 0x76 + 0x10 | | |
