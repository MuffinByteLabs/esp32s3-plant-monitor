# Bring-Up Progress — Rev A, Board #1

*Living status doc. Started 2026-09-08, last updated 2026-09-13.*
*Companion to [`BringUp_Guide.md`](BringUp_Guide.md) — the guide is the procedure, this is where board #1 actually got to.
Detailed measurements and the numbered observations live in the guide's §5 record sheet.*

---

## RESUME HERE

**State as of 2026-09-13.** Step 11 is closed — real dry/wet numbers taken on this probe and cable, and
`config.h` updated. The board has since run an 11-hour battery soak (249 wake cycles, no resets) and now
runs the full cycle on a 30-minute interval, publishing to ThingSpeak channel 3492276. Steps 9 and 12
are the outstanding ones and both still need gear that is not on the bench.

The note below was written 2026-09-09 and the procedure is still correct, but **the cell is no longer in
the 3.7–3.9 V window the test wants** — it will have to be caught mid-charge again, or run down.

**Board is charging, cell is at ~4.0 V and climbing. The next action is step 9, and it is time-sensitive.**

Step 9 wants the battery at **3.7–3.9 V**, because the notch bottoms at roughly VBAT − 0.65 V and the test
only means something when that lands near the LDO's limit. At 4.2 V the notch bottoms around 3.55 V and
proves nothing. Every minute on the charger makes the test weaker.

**The trap:** the only way to stop charging is to pull USB, and *pulling USB is the step 9 event itself*.
There is one clean shot per charge state. So:

1. Scope probe on the pad marked **`VSYS`**, ground clip to **`GND`**. DC coupled.
2. ~50 ms/div, single-shot trigger, falling edge, level a few hundred mV below the resting VSYS (~4.7 V).
3. Serial monitor open so the reset reason is visible afterwards.
4. **Then** pull USB.

**PASS:** VSYS steps down to ≈ VBAT − 0.65 V, rides there **≤ ~100 ms**, snaps up to ≈ VBAT and stays.
Board keeps running, no reset, `3V3` never drops below ~3.0 V.
**FAIL:** notch lasts ~0.5–1 s and the board reboots → R16 is not the 10 k part, or the gate node has extra
capacitance. (R16 was already verified at ~10 kΩ cold — see step 2 below — so a fail here would be surprising.)

After step 9: step 10 (battery-only cold boot), then let it charge to termination to close out step 7.

---

## Status by step

| Step | What | Status |
|---|---|---|
| 1 | Visual inspection | **PASS** — all clear |
| 2 | Resistance / shorts, no power | **PASS** — all four rails in range |
| 3 | Rails on USB | **PASS** |
| 4 | USB enumeration + first flash | **PASS** — blank-flash boot loop diagnosed, not a fault |
| 5 | I²C scan | **PASS** — 0x76 and 0x10 both found |
| 6 | Sensor sanity | **PASS** |
| 0.1 | Battery plug polarity ritual | **PASS** — red = +, aligns with J3 pin 1, pack is protected |
| 7 | Battery connect + charge | **IN PROGRESS** — charging confirmed; termination check outstanding |
| 8 | BAT_SENSE ratio | **PASS** — ratio 0.4975; `BAT_TRIM` measured 2026-09-09 = **1.000** |
| 9 | Hand-over notch (scope) | **NEXT — do this now** |
| 10 | Battery-only cold boot | Pending |
| 11 | Soil calibration | **PASS** — dry/wet/threshold recorded 2026-09-13; `config.h` constants replaced |
| 12 | Sleep current | **BLOCKED** — still needs a µA meter. Partial: whole-cycle draw now derived from the battery curve, see finding 8 |
| 13 | Wi-Fi range + 24 h soak | **PARTIAL** — 11 h soak 2026-09-12/13, 249 cycles, counter unbroken. Bench RSSI −34 to −48 dBm. Range at the plant's location not yet checked |

## Measurements recorded

| Item | Expected | Measured | Verdict |
|---|---|---|---|
| +5V_PROT ↔ GND, cold | ≈10 kΩ (R16 — *this is the R16 value check*) | in range | PASS |
| VSYS ↔ GND, cold | MΩ / OL | in range | PASS |
| +3V3 ↔ GND, cold | MΩ | in range | PASS |
| VBAT ↔ GND, cold | ≈200 kΩ (R14+R15) | in range | PASS |
| +5V_PROT (USB) | 4.87–4.96 V | in range | PASS |
| VSYS (USB, no batt) | 4.45–4.65 V | in range | PASS |
| **VSYS (USB + charging)** | 4.45–4.65 V | **4.7 V** | PASS — see finding 5 |
| +3V3 | 3.30 V ±2 % | in range | PASS |
| Q3 V_GS, USB present | ≈ +0.4 V (Q3 off) | in range | PASS |
| **VBAT (charging)** | rising to 4.20 V | **3.9 → 4.0 V** | PASS |
| **BAT_SENSE ÷ VBAT** | 0.50 ±1 % | **1.99 / 4.00 = 0.4975** (−0.5 %) | **PASS** |
| I²C scan | 0x76 + 0x10 | both found | PASS |
| Charge current | 90–110 mA | *not measured — no inline current meter* | — |
| **BAT_TRIM** | 1.0 ±few % | **1.000** — meter and firmware agreed to the meter's resolution (±~0.3 %) | **PASS** |
| **Soil raw — probe in open air** | — | **2267** | calibration point, defines 0 % |
| **Soil raw — soil that wants watering** | — | **2011** | calibration point, = 24.7 %; sets `ALERT_SOIL_DRY_PCT` at 25 % |
| **Soil raw — soil just watered** | — | **1229** | calibration point, defines 100 % |
| **`SOIL_RAW_DRY` / `SOIL_RAW_WET`** | breadboard ≈2900 / ≈1465 | **2267 / 1229** | **PASS** — breadboard values were unusable, see finding 7 |
| **Wake cycle duration** | — | **4.04 s** — sensors + fast-path association + one POST | from the firmware's own `awake for` line |
| **Wake-cycle average current** | 80 mA (firmware's working assumption) | **≈135 mA** | derived, see finding 8 — the assumption is ~70 % optimistic |

Rail voltages were confirmed in-band but exact figures were not written down. **Record real numbers on
boards #2–#5** — unit-to-unit spread on +3V3 and on D4's drop (5V − VSYS) is the baseline that makes a
Rev B decision easy.

## Findings — things the guide did not say

1. **Silk labels are net names, not TP numbers.** The guide's §2 probe table uses TP1–TP12, but the
   silkscreen reads `5V` `VSYS` `3V3` `VBAT` `BAT_SNS` `SIG` `GND` `EN` `IO0` `TX` `RX`. Mapping:
   TP1=5V, TP2=VSYS, TP3=3V3, TP4=VBAT, TP5=BAT_SNS, TP6=SIG, TP7=GND, TP9=EN, TP10=IO0,
   TP11=TX, TP12=RX, TP8=the GND pad in the UART row of three.
2. **R16 can be verified cold with a multimeter — no scope needed.** It is the *only* resistive path
   from +5V_PROT to GND, so that rail's cold resistance reads its value directly. ~10 kΩ = the fix is
   fitted; ~100 kΩ = the old part. Catches the step 9 failure before power is ever applied.
3. **There is no GPIO-controlled LED on this board.** D2 is hardwired to +3V3 through R5; D3 belongs to
   the charger. Step 4's "flash a hello/blink build" is impossible as written — use a serial hello.
4. **A blank module makes the USB port connect/disconnect about once a second.** Erased flash reads as
   `0xFFFFFFFF`, the ROM finds no application, resets, and retries forever; because USB is the ESP32's own
   peripheral, every reset re-enumerates. Sounds alarming, is normal on a new board. **Fix: hold SW1 (BOOT)
   while plugging USB in**, hold two more seconds, release — that forces ROM download mode, which runs no
   application code and cannot loop. Flash once and it never happens again.
5. **VSYS reads ~4.7 V, above the documented 4.45–4.65 V band.** Not a fault — VSYS is 5V minus D4's forward
   drop, and a Schottky's V_f is smaller at light load than the guide assumed. The band is pessimistic.
6. Observations 1–3 in the guide's §5: humidity self-heating offset, the corrected charger-cycling
   mechanism plus D3-as-duty-cycle-indicator, and the reversed-probe-cable signature.
7. **The breadboard soil constants were not imprecise, they were unusable — and the scale failed
   silently.** `config.h` shipped with `SOIL_RAW_DRY 2900` from the breadboard. This probe on this board
   reads **2267 in open air**, and air is as dry as anything ever gets. Through
   `pct = (DRY − raw) / (DRY − WET) × 100` that put the floor of the scale at
   `(2900 − 2267) / 1435 = 45.7 %`: the reported percentage could not physically go below 45.7 %, so a
   dry-soil alert set at 30 % was unreachable no matter how parched the pot. Nothing warns about this.
   The percentage looks plausible, moves in the right direction, and is simply missing its bottom half —
   which is exactly the failure a week of unattended logging would not reveal. **Never carry a soil
   constant across from a breadboard; the probe, the cable and the board are all part of the divider.**
8. **The battery discharge curve is a current meter, and step 12 does not strictly need one.** An 11-hour
   soak at one-minute wakes took the cell 4.170 V → 4.076 V. Discarding the first ~30 minutes (surface
   charge off the charger, visibly twice as steep as everything after), the settled slope is ≈1.75 %/h,
   which on the 500 mAh pack is **≈8–9 mA average**. With a 4.04 s wake in a 64 s cycle — 6.3 % duty — and
   the documented 210 µA floor assumed for the other 93.7 %, the wake cycle averages **≈135 mA**. That
   figure is robust: doubling the assumed sleep floor only moves it to ≈130 mA. The sleep term is what a
   single whole-cycle measurement cannot separate out — but run the same experiment at a 30-minute
   interval, where the duty cycle is 0.2 %, and the slope is almost entirely sleep. **Two soaks at two
   intervals give both numbers with no µA meter.**

## Guide corrections still to make

- §1: D3 explanation — wording ("flickers faintly") is right, mechanism is not. See §5 observation 2.
- §2: add the silk-label column so the probe table matches the board.
- §3 step 3: VSYS band runs low; consider 4.45–4.75 V.
- §3 step 4: remove "blink build" — no GPIO LED exists.
- §3 step 11: add the reversed-cable signature (high, constant, unresponsive — not dead).
- §3 step 11: state plainly that breadboard dry/wet numbers must not be carried onto the board, and say
  why — the failure is a scale with no bottom, not a wrong reading. See finding 7.
- §3 step 11: take **three** points, not two. Air and water bracket the range, but the number that
  actually matters is the raw value of soil that wants watering — that is what the alert threshold is set
  from, and it cannot be interpolated reliably from the other two.
- §3 section A: add the cold resistance checks as a numbered step, incl. the R16 trick.

## Files added this session

| File | Purpose |
|---|---|
| `firmware/sensor_check/sensor_check.ino` | The one in use. Reads BME280 + VEML7700 + soil + BAT_SENSE, never sleeps so the USB CDC port stays alive. **Correct I²C pins: SDA 38, SCL 39.** |
| `firmware/bringup_check/bringup_check.ino` | Fuller diagnostic — decodes the reset reason by name (incl. `BROWNOUT`), reports chip/flash/PSRAM, scans I²C. Useful for step 9 and step 10 |

**Known gap:** `sensor_check.ino` takes a *single* ADC sample for soil where `config.h` assumes 16.
Average before recording any calibration value.

## Blocked

- **Step 12, sleep current** — needs a meter resolving µA in series with the cell. Rework gear is on the
  bench so D2 can be lifted for the ~90 µA measurement, but nothing can read it. **Partially routed
  around:** finding 8 derives ≈135 mA for the wake cycle from the battery curve. A 30-minute-interval
  soak isolates the sleep term the same way, which would close this without buying anything.
- **Charge current (step 7)** — needs an inline current meter. R13 = 10 k programs 1000/10k = 100 mA by
  design; a sensibly-rising VBAT is decent circumstantial evidence.

## Context a fresh session needs

- **I²C is on IO38/IO39**, re-pinned during layout 2026-08-15. `Wire.begin(38, 39)` — SDA first. The old
  IO4/IO5 numbers appear in retired docs and will silently find nothing.
- **SW1 = BOOT, SW2 = RESET.**
- **Soil probe: GPIO21 LOW = probe ON**, wait ≥200 ms, read GPIO1. Higher raw count = drier.
- **Soil calibration is board #1's, taken 2026-09-13**: air 2267, wants-water 2011, just-watered 1229.
  These belong to this probe, this cable and this board — re-take all three on boards #2–#5.
- **BAT_SENSE lies whenever USB is present with no cell** — the charger holds VBAT near 4.2 V and the
  divider faithfully reports it. Firmware must gate battery reporting on USB absence. Rev A has **no VBUS
  sense line**, so firmware uses "has a host opened the CDC port" as a proxy — right at the bench, wrong on
  a phone charger. Rev B item.
- **Never store the board with a flat cell plugged in.** ~210 µA sleep floor; 0 V recovery is unavailable
  on these packs.
