# Bring-Up Guide — ESP32-S3 Plant Monitor Rev A
*Replaced the original bring-up checklist PDF on 2026-07-20 (it referenced test points and a soil divider that no longer exist); that PDF was retired. Designators match the schematic, not the design doc.*

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

7. **Meter the pack plug** (rule 0.1), then connect at J3 with USB still in. Charge LED on; VBAT (C16) climbs toward 4.20 V; charge current ≈ 90–110 mA (my as-received cell measured 3.95 V, so expect a shortish CC phase then taper).
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

### Board #1 — 2026-09-08 (session in progress)

**Pre-power resistance checks (rail ↔ GND, meter in Ω mode).** Not in the original sequence — added this session because the values are derivable from the board file and one of them verifies R16 without a scope.

| Item | Expected | Derivation | Measured | Pass |
|---|---|---|---|---|
| +5V_PROT ↔ GND | ≈ 10 kΩ | R16 is the only resistive path on the rail — **this reading _is_ the R16 value check** | in range | ✅ |
| VSYS ↔ GND | MΩ / OL | no resistor on the net; D4 and Q3 both reverse-biased, U2 below startup | in range | ✅ |
| +3V3 ↔ GND | MΩ (range-dependent) | R5's path is blocked by D2 in series (LED sub-threshold) | in range | ✅ |
| VBAT ↔ GND | ≈ 200 kΩ | R14 + R15 in series — pre-validates the BAT_SENSE divider | in range | ✅ |

**Visual inspection (step 1):** all clear. C3/C4 correctly unpopulated (DNP), U1 at 45° as designed.

### Main sheet

| Item | Expected | Measured | Pass |
|---|---|---|---|
| +5V_PROT (USB) | 4.87–4.96 V | in range | ✅ |
| VSYS (USB) | 4.45–4.65 V | in range | ✅ |
| +3V3 | 3.30 V ± 2 % | in range | ✅ |
| Q3 V_GS, USB present | ≈ +0.4 V (Q3 off, battery path isolated) | in range | ✅ |
| Charge current | 90–110 mA | | |
| VBAT at termination | 4.168–4.232 V | | |
| BAT_SENSE ÷ VBAT | 0.50 ± 1 % | | |
| Hand-over notch | ≤ ~100 ms, no reset | | |
| Soil dry / wet raw | ≈ 2900 / ≈ 1465 | | |
| Sleep current | ~210 µA (~90 µA no LED) | | |
| I²C scan | 0x76 + 0x10 | **both found** — BME280 0x76, VEML7700 0x10 | ✅ |

### Sensor readings, board #1, 2026-09-08 (USB only, no battery, no probe)

| Reading | Value | Note |
|---|---|---|
| Temperature | 26.42 → 26.62 °C over ~30 s | Rising then flat — U4 self-heating, as expected |
| Humidity | 50.5 % → settles 45.6 % | Falls as the die warms; see observation 1 below |
| Pressure | 1019.2 hPa, ±0.03 | Rock steady. Best evidence the I²C bus is clean |
| Light | ~145 → ~167 lux, ±3 | Room light, sensible response |
| Soil raw | 0 | Correct — nothing in J2, R11 pulls the input down |
| BAT_SENSE | 2411–2453 raw, **cycling** | Not static. See observation 2 below |

**Observation 1 — humidity falls during warm-up, and that is physics, not drift.**
Relative humidity is temperature-dependent. As U4 self-heats ~0.2 °C the air at the
die holds more moisture at saturation, so the same absolute humidity reads as a lower
percentage. The 5-point drop from 50.5 % to 45.6 % settles as the die reaches thermal
equilibrium. Consequence: **this board reads humidity slightly low**, and the offset
is proportional to the self-heating. Worth a fixed correction in firmware, and worth
noting that the LDO's proximity to U4 sets the size of it (Rev B placement item).

**Observation 2 — BAT_SENSE is not static with no battery fitted. Mechanism corrected 2026-09-09.**

*First analysis was wrong and is kept here as a warning.* It claimed a ~12–13 s charge/terminate
cycle, read straight off the serial log. **That period was an aliasing artifact.** The sketch samples
BAT_SENSE about every 1.25 s, and sampling a fast oscillation slowly produces a convincing but fake
slow beat. The physics rules 12 s out: the only load on VBAT with no cell is the R14/R15 divider
(~21 µA at 4.2 V), and 21 µA draining C16's 4.7 µF across a 70 mV window takes roughly **16 ms**, not
6 seconds. Lesson: do not read a frequency off a log sampled near that frequency.

*Real mechanism.* The MCP73831 has **no battery-detection circuit at all** — it never looks for a
battery and is never "confused." It just runs its fixed routine: push 100 mA until 4.2 V, hold, declare
complete when current falls below ~7.5 mA (7.5 % of the 100 mA that R13 programs), auto-recharge when
the voltage later sags. With a 500 mAh cell each stage takes tens of minutes. With only C16's 4.7 µF to
fill and a 21 µA leak through the divider, the identical routine completes in milliseconds and repeats
forever. **The divider is the leak that makes it repeat** — without it the charger would top up once and
stay terminated.

*D3's brightness is the duty cycle, and it is a usable field indicator.* Confirmed on hardware 2026-09-09:

| D3 appearance | State |
|---|---|
| Dim but lit, no visible blink | Fast cycling on an empty node — STAT low only part of the time |
| **Full bright, steady** | Continuously charging a real cell — STAT held low |
| Off | Terminated, or no USB |

So §1's "flickers faintly" wording is right; its *explanation* (STAT tri-states, cosmetic) is not the
whole story. The exact on/off ratio is still unmeasured — **scope BAT_SNS with the cell disconnected**
to settle it. D3 still proves nothing about polarity: rule 0.2 stands unchanged.

Neither observation affects pass/fail. Both are recorded because the firmware gates
battery reporting on USB absence anyway, so the cycling is invisible in normal use.

> **Rail voltages logged as "in range" — exact figures not captured at the bench.** Worth recording real numbers on the remaining four boards: unit-to-unit spread on +3V3 and on D4's drop (+5V_PROT − VSYS) is the kind of baseline that makes a Rev B decision easy later.

**Observation 3 — step 11's "buzz out the probe cable" caught a real fault, and the failure has a specific signature worth knowing.**
The J2 cable was wired **reversed**: board SIG → probe GND, board GND → probe analog out. VCC was
unaffected, because on a 3-pin connector reversed end-for-end **the middle pin maps to itself** — which is
exactly why this is easy to miss and easy to dismiss.

*Signature:* soil reads a **high, constant, unresponsive** value (~2973–2981 here), not a dead one. The probe
still powers up, so the number looks plausible — it just never moves. Mechanism: the probe's ground return
lands on ADC_SOIL and has to reach ground through R11's 100 kΩ, which pins the node near the top of the ADC
range. Contrast with **no probe fitted**, which reads a clean 0.

*No damage.* Everything stayed within 0–3.3 V (nothing on that connector is fed from higher), the probe was
never reverse-powered thanks to the middle-pin property, and R11 limited current to microamps. Verified after
the fix: probe responds normally.

**Step 11 in progress — first readings after the cable fix, not yet calibration-grade:**
probe resting on a desk read ~2265, wet hands read 1209–1911. Neither is valid: a desk has a far higher
dielectric constant than air (biases "dry" low), and a hand couples its own capacitance into the blade
(hence the ~700-count swing). **Calibration must be taken blade-in-free-air and submerged-in-water, hands
off both times.** Also note `sensor_check.ino` takes a *single* ADC sample for soil where `config.h` assumes
16 — average before recording.

**Deferred — gear not on the bench 2026-09-08:** steps 7–10 (battery), step 9 (scope), step 11 (soil probe), step 12 (sleep current). Multimeter-only fallback for step 9: pull USB and read the reset-reason register in the logs — gives pass/fail on the brown-out without measuring notch width.
