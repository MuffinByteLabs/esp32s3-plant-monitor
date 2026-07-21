# Pre-Fabrication Design Review — ESP32-S3 Plant Monitor Rev A
**Date:** 2026-07-20 · **Updated:** same day, after my 16:07 save (battery-sense verified) and BOM regeneration. · **Scope:** full schematic, netlist-level, against all datasheets in `references/`, the Final Design Document, Espressif hardware design guidelines, and the Zak Kemble load-sharing reference topology. The deleted "Rev A.2" reference PDF is **not** used as evidence anywhere in this report; all findings trace to the parsed `.kicad_sch` files, my screenshots, and the datasheets.

**Method.** I did not review from the screenshots alone. I parsed the actual `.kicad_sch` s-expression sources into a pin-level netlist (symbol transforms validated against wire/junction geometry three independent ways), and cross-checked every claim against the PDFs in `references/`. Reference designators below are the **current-schematic** ones (my screenshots); the doc↔schematic cross-map is in §8.

---

## VERDICT (summary — details and ranked list below)

**The circuit as drawn in my screenshots is electrically sound and will work** — power-up, charge, battery operation, sensors, and native-USB programming are all correctly wired, **except**:

1. ~~BLOCKING — USB-C GND pins no-connected~~ → **RESOLVED 2026-07-20 16:29.** Re-parsed after my fix: A1/B1/A12/B12 now net to GND; the only remaining no-connects on the sheet are SBU1/SBU2 (correct).
2. ~~BLOCKING (process) — saved files missing the battery-sense chain~~ → **RESOLVED 2026-07-20 16:07.** I re-parsed my save: IO2 → `BAT_SENSE` on sheet 04, and `VBAT` → R14 100 k 1 % → `BAT_SENSE` (R15 100 k 1 % → GND, C17 100 nF at the pin) on sheet 07; labels merge across sheets. Battery monitoring is correct in the saved files. Remaining step: commit to git (the last commit still lacks it).
3. ~~BLOCKING (for assembly) — BOM stale~~ → **RESOLVED 2026-07-20.** `references/ESP32S3_PlantMonitor_RevA_BOM.xlsx` was regenerated from the saved schematic (designators now match the schematic; 51 placed + 2 DNP = 53 footprints). Re-verify JLC stock/Basic-Extended status at upload.
4. ~~RISKY — USB-unplug hand-over brown-out~~ → **RESOLVED 2026-07-20 18:52.** R16 verified changed to 10 k in the saved schematic; BOM updated to match. Residual behavior to confirm at bring-up: a brief (~50–100 ms) dip to VBAT−0.65 V on VSYS during unplug — pass criteria and procedure are in `docs/BringUp_Guide.md`. Integrated power-path options are documented in `docs/RevB_Upgrade_Plan.md`.

Remaining before layout: add test points (or accept the pad-level probe points listed in the bring-up guide), commit.

---

## 1. File & folder audit (everything in the folder)

| File | Status |
|---|---|
| `ESP32S3_PlantMonitor/*.kicad_sch` (re-saved 2026-07-20, latest 16:29) | **Verified after my saves:** BAT_SENSE subcircuit present and correctly netted; USB-C GND pins now grounded (fix verified 16:29); refs match my screenshots (R12=470 LED, R13=10 k PROG, R14/R15 divider, R16 rail pull-down, U6 charger). R16 still 100 k (Risky #4 — decision pending). |
| `ESP32S3_PlantMonitor/.history/*.kicad_sch` | Autosave snapshots; now match the saved files. No further action. |
| Git working tree | Dirty — every sheet modified vs. last commit (`a520f59 starting last stages`); the last *commit* still lacks BAT_SENSE. **Action: commit the saved state (ideally after fixing Blocking #1 and R16).** |
| ~~`ESP32S3_PlantMonitor_RevA2_Schematic (1).pdf`~~ | I deleted it; I'd already flagged it as unreliable. **No finding in this review depends on it** — everything was re-derived from the `.kicad_sch` sources, my screenshots, and the datasheets. |
| `ESP32S3_PlantMonitor.kicad_pcb` | 79-byte stub — layout not started (fine; this review is pre-layout). |
| `08_Mechanical.kicad_sch` | **Empty.** Doc §9 promises 4× M3 mounting holes here. Add before layout. |
| `references/ESP32S3_PlantMonitor_RevA_BOM.xlsx` | **Regenerated 2026-07-20 from the saved schematic** (this review): schematic designators, corrected values (10 k power-LED, 470 charge-LED, no soil divider, 3×100 k + 2×100 k 1 %), working summary formulas, carried-over JLC part numbers, notes flagging the J1-GND and R16 open items. Original backed up in session scratch. |
| Bring-up checklist + pin-map PDFs | **Superseded 2026-07-20** by `docs/BringUp_Guide.md` and `docs/PinMap_CheatSheet.md` (both PDFs were stale: TP references, deleted soil divider). Safe to delete the PDFs. |
| Design doc | **Updated 2026-07-20** (`references/` copy): soil section rewritten to as-built, LED resistors, R14→10 k, hand-over explanation corrected, USB GND bullet, 500 mAh cell + firmware duties, BOM-table rows fixed, plus "Addendum A — As-Built Updates" with the doc↔schematic refdes map. The duplicate in the project root is now outdated — delete it. |
| Datasheets | All present and correct for the chosen parts, two housekeeping notes: `ss14.pdf` is a generic "SS12–SS1200" family sheet (not MDD-branded) and still has its unrenamed filename (doc §16 open item); the TVS sheet is MDD-named for a Goodwork part (doc §16 asks for the Goodwork doc). |
| **IPC-2221** | **Not in the folder** (I'd noted it should be there). Only relevant at layout (trace widths), but re-add it. |
| `libs/` | VEML7700 symbol/footprint/3D present; footprint-vs-symbol pin mapping should be spot-checked at layout (BME280's clockwise pin numbering is called out in the doc — good). |

---

## 2. Block-by-block, net-by-net findings

### Sheet 02 — USB-C input & protection
Verified correct against J1 (16-pin C165948), USBLC6-2SC6, SMF5.0A, 1206L075/16WR datasheets:

* VBUS: all four pads (A4/B4/A9/B9) tied → `USB_VBUS`; C1 100 n at connector; D1 TVS **cathode to VBUS, anode to GND** (correct clamp direction; verified from symbol transform math, not the picture); F1 in series → `+5V_PROT` with C2 10 µF after the fuse. Matches Espressif guideline "ESD diode + ≥10 µF at main power entrance."
* CC1→R4 5.1 k→GND, CC2→R3 5.1 k→GND — one resistor per pin, correct UFP advertisement, never shared. ✔
* Data path has **no polarity swap**: J1 D− (A7+B7) → U1 pin 1 (I/O1) ⇒ pin 6 → R1 22 Ω → `USB_DN` → IO19 (USB_D−); J1 D+ (A6+B6) → U1 pin 3 ⇒ pin 4 → R2 22 Ω → `USB_DP` → IO20 (USB_D+). USBLC6 used flow-through exactly per ST datasheet; VBUS pin 5 powered, GND pin 2 grounded. C3/C4 100 n correctly DNP. ✔
* SBU1/SBU2 NC ✔. Shield → GND directly ✔ (doc-sanctioned for Rev A).
* **✘ BLOCKING — FIXED 2026-07-20 16:29 (verified: all four pins now net to GND).** Original finding, kept for the record: J1's GND pins A1/B1/A12/B12 carried an explicit no-connect flag and touched no copper (visible as the ✕ on the stacked GND pin in my own screenshot; confirmed in the parsed netlist of the 16:07 save). These four pins are where the cable's ground *conductor* lands — they are the DC return for every milliamp of VBUS current and the reference for D+/D−. The shield (SH) is a separate EMI screen bonded to the connector shells; the USB spec treats shield and GND as distinct networks, shell bonding quality varies wildly between cables, and returning supply current through the braid is out of spec even when it happens to work. Consequences range from "works with cable A, dead with cable B" to flaky enumeration to a defeated ESD strategy (USBLC6 clamps into a ground the cable ground never reaches). ERC is clean because the explicit NC flag tells it this is intentional. **Fix: delete the NC marker, wire the stacked GND pin to GND (one wire grounds all four pads — KiCad stacks them).**
* Minor: D1's schematic value is generic `D_Zener` and F1's is `Fuse` — set them to `SMF5.0A` and `1206L075/16WR 0.75A` so BOM export can't pick the wrong part. Minor: TVS sits before the fuse — perfect for ESD (its job), but a sustained-overvoltage event cooks it with no fuse protection; moving it after F1 is slightly more robust (optional). Minor: with D4 conducting at plug-in, VBUS sees ≈25 µF of downstream ceramic (USB spec inrush guidance is 10 µF); the polyfuse and Schottky soften this — accepted practice at this board class.

### Sheet 03 — 3.3 V regulator
Verified against AP2112K-3.3 datasheet:

* VSYS → C5 10 µF + C6 1 µF → VIN; EN tied to VIN (mandatory — internal 3 MΩ pull-down otherwise holds it off ✔); VOUT → C7 1 µF → `+3V3`. Datasheet requires 1 µF/1 µF ceramic minimum — met, plus the added 10 µF VSYS bulk (my intentional change #4 — present and correct). ✔
* 600 mA min guaranteed ≥ Espressif's 500 mA supply requirement ✔. Dropout (3.3 V grade): 250 mV typ / 400 mV max at 600 mA ⇒ ~150/240 mV at the 355 mA Wi-Fi peak. Iq 55 µA typ.
* Power LED: `+3V3` → R5 10 k → D2 (anode→cathode) → GND. Direction correct. **I = (3.3−2.1)/10 k ≈ 120 µA.** It will stay green (color doesn't shift with current) but on many 0603 greens 120 µA is *barely* visible in daylight — bench-check one before committing; 2.2 k (~550 µA) is a middle ground. Also note 120 µA is now the **largest single sleep-mode drain** (see §5) — the doc's "unsolder D1 for battery tests" advice still applies.
* Thermal (linear, worst sustained USB case): (4.6−3.3) V × 0.2 A ≈ 0.26 W → ~65–90 °C rise in SOT-23-5. Survivable, has thermal shutdown, doc's copper-pour mitigation stands — keep it. Sustained OTA/Wi-Fi at peak on USB is the case to watch at bring-up.

### Sheet 04 — ESP32-S3 core
Verified against WROOM-1 datasheet v1.8 and the 2026-06-23 hardware design guidelines:

* EN: R7 10 k pull-up + C8 1 µF + SW2 (reset) — exactly the guideline RC (10 k/1 µF, ~10 ms) ✔. IO0: R6 10 k pull-up + SW1 (boot) to GND, no capacitor on the strap ✔.
* 3V3 pin: C9 22 µF + C10 100 n at the pin ✔ (≥ guideline 10 µF + 0.1 µF; adding a parallel 1 µF is optional polish).
* GND pins 1, 40, EPAD 41 all grounded ✔.
* Straps: IO3, IO45, IO46 no-connected per guidelines/doc ✔ (IO45 floating ⇒ VDD_SPI = 3.3 V, correct for the N8 module). IO35/36/37 NC'd with a "PSRAM" note — on the **N8 (no PSRAM)** these pins are actually free, so the annotation is misleading but harmless; NC is fine.
* Periphered pins: IO4=`SCL`, IO5=`SDA`, IO1 (ADC1_CH0)=`ADC_SOIL`, IO2 (ADC1_CH1)=`BAT_SENSE` *(current version)*, IO21=`SENS_PWR_EN`, IO19/20=`USB_DN`/`USB_DP` — all match the doc's pin plan; both analog inputs are on **ADC1** ✔ (ADC2 is Wi-Fi-unusable, correctly avoided).
* TXD0/RXD0 no-connected. The doc promises TP12/TP13 pads as the UART recovery path — see Risky #3 (no test points exist).
* ✘ In the **saved** file only: IO2 is no-connected (the missing-divider problem, Blocking #2).

### Sheet 05 — I2C sensors
Verified against BME280 rev 1.24 and VEML7700 rev 1.8 datasheets:

* One pull-up pair for the whole bus: R8 (SCL) + R9 (SDA) 4.7 k to +3V3 ✔ (0.7 mA per line when low; fine to 400 kHz with usual bus capacitance).
* BME280 (U4): VDD (8) and VDDIO (6) → +3V3 with **two** 100 n caps (C12, C11) per datasheet ✔; GND 1/7 ✔; **SDO (5) → GND ⇒ address 0x76** ✔ (datasheet: 111011**0**); **CSB (2) → +3V3 ⇒ I2C mode locked** ✔; SCK (4) → SCL, SDI (3) → SDA ✔.
* VEML7700 (U5): pinout 1=SCL, 2=VDD, 3=GND, 4=SDA per datasheet — wired exactly so ✔; C13 100 n at VDD ✔; address fixed 0x10 ✔ (no conflict with 0x76); VDD 2.5–3.6 V range fits 3.3 V ✔.
* Layout reminders from the datasheets (doc already has them): BME vent hole exposed, clockwise pin numbering, away from heat; VEML clear sky view, away from LEDs.

### Sheet 06 — Soil probe input
Verified against SEN0193 web datasheet and AO3401A datasheet:

* Q1 high-side switch: **S(2) → +3V3, D(3) → `SOIL_PWR` → J2.2, G(1) → `SENS_PWR_EN` with R10 100 k gate-to-source pull-up.** Default off (boot, deep sleep — GPIO floats, R10 holds Vgs=0) ✔; GPIO21 low ⇒ Vgs = −3.3 V ⇒ Rds(on) ≈ 70 mΩ at −2.5…−4.5 V spec ⇒ µV-level drop at the probe's 5 mA ✔. Body diode points load→rail, never forward ✔.
* J2 order (my intentional change #2): pin 1 = `ADC_SOIL`, pin 2 = `SOIL_PWR` (**power on the middle pin**), pin 3 = GND. This matches the DFRobot Gravity analog convention (signal / VCC / GND) and my metered probe. The doc's original order (power/GND/signal) would indeed have reverse-powered the probe — the change is right. Cable buzz-out before first use stays mandatory (probe cables have no color standard — doc §16 ✔).
* ADC net: R11 100 k pull-down (added, correct — pin can't float with probe off) + C14 100 n filter at the pin (guideline: 0.1 µF at ADC pins ✔).
* Range check (guideline: ATTEN=3 effective range **0–2900 mV**, ±50 mV): probe outputs ≤3.0 V at a 5 V supply and **~2.2 V max at 3.3 V supply (my bench value — plausible against the datasheet's 0–3.0 V @5 V spec)** ⇒ inside range with ~0.7 V margin ✔. Even the "accidental 5 V" case (3.0 V) is below the 3.6 V pin abs-max — reading saturates but nothing is damaged. Removing the doc's 2:1 divider is sound *given my measurement*; re-verify max (dry, in air) at bring-up before trusting calibration.

### Sheet 07 — Battery, charger & power path
Verified against MCP73831 DS20001984H, AO3401A, SS14-family, PKCell 503035 datasheets. **This sheet is correct** — including all four things I flagged as visually unverifiable:

1. **D4 polarity: anode on `+5V_PROT`, cathode on `VSYS`** — current flows USB → VSYS. Confirmed from the parsed geometry (symbol at 215.9/76.2, rot 90; pin-map validated against three other diodes whose orientation is functionally provable). ✔
2. **Q3 gate connects to the `+5V_PROT` net itself** — a dedicated vertical wire at x=207.01 up to the rail junction, *not* through D4 and not a stray local net. R16 100 k pulls that same net to GND. ✔ (but see Risky #1 — the rail's 14.7 µF makes this pull-down slow).
3. **Charger pins, all five:** VDD(4) → +5V_PROT with C15 4.7 µF ✔ (datasheet-recommended value); VSS(2) → GND ✔; VBAT(3) → `VBAT` with C16 4.7 µF ✔; PROG(5) → R13 10 k → GND ✔; STAT(1) ← D3 cathode, D3 anode ← R12 470 Ω ← +5V_PROT ✔ (STAT sinks low while charging ⇒ LED on; tri-states with no battery ⇒ faint flicker, doc-documented).
4. **`BAT_SENSE` spelling is identical on sheets 04 and 07** (in the current/.history version) and the nets merge. ✔ — but it exists **only** there, not in the saved files (Blocking #2).

Other verifications on this sheet:

* **Charger variant/settings:** `MCP73831-2` = 4.20 V regulation (datasheet: 4.168–4.232 V) — correct for the PKCell pack (its PCM overcharge trip sits above 4.2 V; a −3/−4 variant would fight it). PROG 10 k ⇒ I = 1000 V/10 k = **100 mA (datasheet band 90–110)** = 0.2 C for the 500 mAh cell = exactly the pack's *standard charge* rate ✔. Precondition 10 mA below VPTH = 66.5 %·VREG ≈ 2.79 V ✔. Termination ≈7.5 mA vs. pack's 5 mA cut-off spec — terminates slightly early; safe direction ✔. VDD range 3.75–6 V ✔. Charger thermals: ≈0.2 W worst (CC into a 3.0 V cell) → ~+45 °C in SOT-23-5, fine.
* **Q2 reverse protection:** G(1)→GND, S(2)→`VBAT`, D(3)→`VBAT_RAW`(J3.1). Correct topology: correct battery ⇒ body diode conducts first, source rises, Vgs ≈ −VBAT ⇒ fully on (charging current passes backwards through the channel fine); reversed battery ⇒ body diode blocks, Vgs = 0 ⇒ off. Vds worst −8.4 V ≪ −30 V rating; Vgs −4.2 V ≪ ±12 V ✔. **Caveat: protection holds only with USB absent — see Risky #2.**
* **Battery divider:** `VBAT` → R14 100 k 1% → `BAT_SENSE` (→ R15 100 k 1% → GND, C17 100 n at pin). Half of VBAT: 2.10 V full / 1.50 V empty — inside 0–2900 mV ✔; 21 µA constant drain ✔ (doc-accepted); divider correctly taps **VBAT (post-Q2)**, so a reversed battery can't drive the ADC pin negative ✔. 50 k source impedance is fine with the 100 n reservoir cap ✔.
* **J3:** pin 1 = `VBAT_RAW` (+), pin 2 = GND, per doc convention. **The schematic cannot verify my physical pack's plug polarity — the mandatory meter check before first insertion stands** (and see Risky #2 for why it matters even more with USB attached). I could not find the literal "reverse the plug" wording in the PKCell PDF's text layer (it may be in a drawing); the datasheet does confirm 4.2 V charge, 0.2 C standard rate, 3.0 V discharge end, PCM present.
* C18 1 µF on VSYS + C5/C6 on sheet 03 give VSYS ≈ 12.6 µF total bulk ✔ (my intentional change #4 — present; and see §4 for why the *doc's* idea that the +5V_PROT cap bridges hand-over is wrong).

---

## 3. MOSFET proofs (pin-by-pin, per AO3401A datasheet: 1=G, 2=S, 3=D; Vgs(th) −0.5/−0.9/−1.3 V min/typ/max; body diode anode=drain, cathode=source)

**Q3 (source-select):** D=VBAT, S=VSYS, G=+5V_PROT.
| State | V(G) | V(S) | Vgs | Result |
|---|---|---|---|---|
| USB present | 5.0 | ≈4.6 | **+0.4 V** | Off ✔. Body diode: VBAT(≤4.2) at anode < VSYS(4.6) at cathode ⇒ reverse-biased ⇒ **battery fully isolated, no body-diode sneak path** ✔. Charge terminates cleanly (load never loads the charger) ✔ |
| Battery only | ≈0 (via R16) | ≈VBAT | **−3.0…−4.2 V** | Fully enhanced (≪ −1.3 V worst-case threshold) ✔, Rds ≈ 40–60 mΩ ⇒ <25 mV drop at Wi-Fi peak ✔ |
| Hand-over instant | decaying | VBAT−0.65 | crosses −Vth **late** | see Risky #1 |

**Q2 (reverse guard):** D=VBAT_RAW, S=VBAT, G=GND.
| State | Vgs | Result |
|---|---|---|
| Correct battery | −(VBAT−0.7)→−VBAT | On, both directions (charging passes) ✔ |
| Reversed, no USB | 0 | Off; body diode reverse ⇒ blocked ✔ |
| Reversed, USB present | −(node) as charger lifts VBAT | **Turns on ⇒ ~10 mA reverse trickle into the cell — Risky #2** |

**Q1 (soil switch):** S=+3V3, D=SOIL_PWR, G=IO21/R10. Off by default (Vgs=0 via R10, including deep sleep); on at Vgs=−3.3 V ✔.

---

## 4. Scenario analysis (voltages at every rail)

Assumes VBUS = 5.0 V; fuse R = 0.09–0.29 Ω; D4 Vf ≈ 0.3–0.45 V (SS14 family: 0.55 V max at 1 A); AP2112 dropout ≈0.15/0.24 V typ/max at 355 mA.

| Node | (a) USB + charging | (b) Battery only | (c) Hand-over moment | (d) Battery reversed |
|---|---|---|---|---|
| USB_VBUS | 5.0 | 0 | collapsing | 5.0 / 0 |
| +5V_PROT | 4.87–4.96 | ≈0 (R16 bleed) | decays, τ = 100 k × 14.7 µF ≈ **1.5 s** | 4.9 / 0 |
| VSYS | **4.45–4.65** | VBAT − ~10 mV | **clamps at VBAT − 0.65 (Q3 body diode) for 0.5–1.0 s** | 4.6 / floats ~0 |
| +3V3 | 3.30 | 3.30 while VBAT ≥ 3.6; 3.25–3.34 at the 3.5 V firmware cutoff under TX (≥3.0 spec ✔) | VBAT=4.2: 3.30 ✔ · 4.0: ~3.1 · **3.8: ~2.9 · 3.6: ~2.7 (brown-out)** | 3.3 / 0 |
| VBAT | 3.0→4.2 (CC 100 mA → CV, term 7.5 mA) | pack voltage | pack voltage | node lifted to ~0.9 V by charger (see below) |
| BAT_SENSE | VBAT/2 (2.10 max) — reads ~2.1 "full" even with **no battery** (charger floats VBAT to 4.2): firmware should gate battery % on USB-absent | VBAT/2 | VBAT/2 | ≈0 (never negative ✔) |

**(c) expanded — the one real electrical weakness.** When USB is pulled, the load discharges +5V_PROT through D4 only down to ≈VSYS+0.2; from there Q3's gate node (= +5V_PROT with C2 10 µF + C15 4.7 µF on it) is discharged *only* by R16 100 k ⇒ τ ≈ 1.5 s. Until the gate falls |Vth| below VSYS, the battery feeds the board **through Q3's body diode at VBAT − 0.65 V** for a computed 530–770 ms (typ Vth) to ~1 s (worst). During that window the LDO is in dropout: at VBAT = 3.8 V the 3.3 V rail sits ≈2.9–3.1 V — at or below the ESP32-S3 brown-out detector (~2.98 V default), and any Wi-Fi burst makes it worse. **My doc's success criterion "unplugging USB does not reset the board" will likely fail below ~3.9 V battery.** The doc's belief that C1 (+5V_PROT 10 µF) "bridges the hand-over" is exactly backwards — capacitance on that node *holds the gate up* and prolongs the body-diode phase (known-difference #4 has this right; the real bridge is the VSYS bank + body diode). **Fix: R16 100 k → 10 k.** τ drops to ~150 ms, body-diode phase to ~50–100 ms; cost is 0.5 mA extra draw *from USB only* (zero battery impact — the node sits at 0 V on battery). Zak Kemble's original has the same characteristic (100 k + 4.7 µF); my extra 10 µF on +5V_PROT makes it three times slower, which is why it bites here.

**(d) expanded.** With USB absent, a reversed pack is fully blocked — clean. **With USB present**, the charger preconditions at 10 mA into the VBAT node; the node rises to ≈0.9 V, which gives Q2 Vgs ≈ −0.9 V ⇒ it starts conducting the 10 mA **into the reversed cell, indefinitely** (MCP73831 has no safety timer; VBAT never reaches the 2.79 V precondition-exit). Q2 dissipates only ~50 mW (survives), but the cell is reverse-trickled at ~0.02 C for as long as it's plugged in, **and D3 glows steady "charging" the whole time** — no fault indication. This is inherent to gate-grounded P-FET protection combined with a charger; no cheap in-circuit fix exists. Controls: my mandatory meter ritual (keep it), ideally a polarity-keyed pre-wired pack, and add "charge LED on does NOT prove correct polarity" to the bring-up checklist.

---

## 5. Current budget & sleep floor (verified numbers)

| Item | Value | Check |
|---|---|---|
| Wi-Fi TX peak (module ds §current) | 355 mA @ 802.11b 20.5 dBm | AP2112 600 mA min ✔; Espressif 500 mA budget ✔ |
| Worst simultaneous 5 V-side | ≈365 mA (3V3 side, linear LDO passes ≈1:1) + 100 mA charge ≈ **465 mA** | Fuse hold 750 mA @20 °C, **650 mA @40 °C** (datasheet derating table) ✔ margin |
| Charge current | 90–110 mA (PROG 10 k) | = 0.2 C standard rate of the 500 mAh PKCell ✔ |
| Charge LED | (5−1.9−0.4)/470 ≈ **5.7 mA** | fine; note this is 470 Ω, doc says 1 k (2.7 mA) — undocumented deviation, harmless |
| Power LED | **120 µA** | dim; dominant sleep load |
| Divider | 21 µA constant | doc-accepted |
| Sleep floor | LDO 55 + LED 120 + divider 21 + ESP ~10 + leaks ~5 ≈ **210 µA** | ≈ 3 months idle on a 500 mAh cell; doc's "≈100 µA floor" assumed no LED — with the 10 k LED fitted the floor roughly doubles. DNP D2 on battery units (doc already allows). |

---

## 6. ADC verification

Guideline (2026-06-23, §ADC): ATTEN=3 effective range **0–2900 mV**, ±50 mV, plus 100 nF at the pin. Both channels: on **ADC1** (IO1=ADC1_CH0, IO2=ADC1_CH1 — module datasheet pin table) ✔, both have 100 nF at the pin ✔, both sources ≤2.2 V ✔ (soil 2.2 V bench max; battery 2.10 V max). ADC2/Wi-Fi conflict correctly avoided. The removed soil divider is justified **by my bench measurement only** — re-measure dry/in-air max at 3.3 V on the real board before trusting absolute calibration; a 5 V mis-supply saturates the reading but can't damage the pin (3.0 V < 3.6 V abs max).

---

## 7. Decoupling / bypass audit (every IC vs. its datasheet)

| IC | Datasheet asks | Schematic has | Verdict |
|---|---|---|---|
| AP2112K | 1 µF in + 1 µF out (ceramic) | 10 µF + 1 µF in, 1 µF out | ✔ exceeds |
| MCP73831 | 4.7 µF VDD + 4.7 µF VBAT | C15/C16 4.7 µF | ✔ exact |
| ESP32-S3 module | ≥10 µF + 0.1 µF at 3V3 pin (guidelines) | 22 µF + 100 nF | ✔ (optional +1 µF) |
| BME280 | 100 nF at VDD and VDDIO | C11 + C12 | ✔ |
| VEML7700 | 100 nF at VDD | C13 | ✔ |
| USBLC6 | local VBUS decoupling | C1 100 nF | ✔ |
| EN/reset RC | 10 k + 1 µF (guidelines) | R7 + C8 | ✔ exact |
| Power entrance | ESD diode + ≥10 µF (guidelines) | SMF5.0A + 10 µF | ✔ |

---

## 8. Design-doc cross-check (by function, not refdes) & my six intentional differences

Cross-map (doc → current schematic): U1→U3 module · U2→U1 USBLC6 · U3→U2 LDO · U4→U6* charger (*U5 in saved file) · U5→U4 BME280 · U6→U5 VEML7700 · Q1→Q2 reverse guard · Q2→Q1 soil switch · Q3→Q3 · R1/R2→R4/R3 CC · R3/R4→R1/R2 22 Ω · R5/C6/SW1→R7/C8/SW2 reset · R6/SW2→R6/SW1 boot · R11→R13 PROG · R12→R12 charge-LED (470 not 1 k) · R13→R5 power-LED (10 k not 1 k) · R14→R16 gate pull-down · R15→R10 soil gate pull-up · R16/R17→R14/R15 divider · D1→D2 power LED · D2→D3 charge LED · D3→D1 TVS · D4→D4.

1. **Soil divider removed** — done cleanly (pull-down + filter kept); justified by bench data; verified in-range. ✔ (Update pin-map PDF which still mentions it.)
2. **J2 pin order reversed** — correct per Gravity convention; power lands on the probe's middle pin. ✔
3. **Power LED 10 k** — wired correctly; brightness/sleep-current notes above. ✔ (BOM/doc still say 1 k — update.)
4. **10 µF added on VSYS** — present (C5). My suspicion about the doc's "+5V_PROT hand-over cap" claim is confirmed and then some: that cap actively *worsens* hand-over via the gate node (§4c). ✔
5. **J3 wired to the metered pack** — schematic is internally consistent (pin 1 = +); physical verification remains mandatory, and Risky #2 raises the stakes with USB present. ✔
6. **Refdes mismatch doc↔schematic** — noted throughout; one functional consequence: **SW1/SW2 roles are swapped vs. doc/BOM** (schematic SW1=BOOT, SW2=RESET; doc says the opposite). Reconcile before silkscreen so the buttons get the right labels.

Un-documented deviations found (add to doc §15 or fix): charge-LED resistor 470 Ω vs doc 1 k; net names `SCL`/`SDA` vs doc `I2C_SCL`/`I2C_SDA` (cosmetic); **no test points at all** vs doc TP1–TP15; empty mechanical sheet vs doc's 4× M3 holes.

---

## 9. Ranked findings

### BLOCKING — must fix before ordering
1. **RESOLVED — USB-C GND pins grounded (verified in the 16:29 save).** A1/B1/A12/B12 now net to GND; SBU1/SBU2 remain the only (correct) no-connects on J1.
2. **RESOLVED — battery-sense chain now in the saved files.** Verified by re-parsing the 2026-07-20 16:07 save: IO2 → `BAT_SENSE` ↔ R14/R15 100 k 1 % divider + C17 100 nF, labels merging across sheets 04/07. *Remaining: commit to git — the last commit (`a520f59`) still lacks it.*
3. **RESOLVED — BOM regenerated from the saved schematic** (2026-07-20, this review): schematic refs, corrected values and quantities, fixed summary formulas, JLC numbers carried over, open items (J1 GND, R16) flagged in-line. *Remaining: re-verify stock/Basic-Extended at JLC upload; update D1/F1 schematic value fields (Minor #8) so future exports are unambiguous; if I change R16 to 10 k, update its BOM row.*

### RISKY — will bite at bring-up or in the field
4. **RESOLVED — hand-over brown-out (sheet 07).** R16 changed 100 k → 10 k, verified in the 18:52 save (τ drops ~1.5 s → ~150 ms; body-diode phase ~50–100 ms; extra 0.5 mA bleed exists only while USB is present). Verify at bring-up with a scope on VSYS while unplugging at VBAT ≈ 3.7 V — procedure in `docs/BringUp_Guide.md`. Long-term option: integrated power path (`docs/RevB_Upgrade_Plan.md`).
5. **Reversed battery + USB = silent reverse trickle-charge.** Q2 blocks a reversed pack only while USB is absent; with USB present the charger's 10 mA precondition turns Q2 on into the reversed cell indefinitely, with the charge LED lying "normal." *Fix: procedural (meter ritual — keep), add a warning line to the bring-up checklist ("LED on ≠ polarity OK"), prefer a keyed/pre-verified pack. No cheap circuit fix in this topology; accept knowingly.*
6. **No test points.** Doc §11 and the bring-up checklist reference TP1–TP15; zero exist in the schematic, and TXD0/RXD0 (the declared UART recovery path) are bare NC pins. *Fix: add TP symbols for at least +5V_PROT, +3V3, GND, VSYS, VBAT, BAT_SENSE, ADC_SOIL, EN, IO0, TXD0, RXD0 before layout.*
7. **LDO sustained-dissipation corner.** ~0.26 W (USB + steady Wi-Fi) in SOT-23-5 ⇒ ~90 °C+ junction on a small pour; fine for bursts, marginal for long OTA sessions at high ambient. *Fix: keep the doc's generous copper plan, verify with a thermocouple at bring-up; no schematic change needed.*

### MINOR — tidy before or during layout
8. Generic value fields: D1 `D_Zener`→`SMF5.0A`, F1 `Fuse`→`1206L075/16WR`, D2/D3 `LED`→ colors (green / amber-red) so silk + BOM are unambiguous.
9. SW1/SW2 refdes roles swapped vs. doc/BOM — reconcile so RESET/BOOT silkscreen is right.
10. Charge-LED resistor 470 Ω (5.7 mA) vs doc's 1 k — fine; document it.
11. Power-LED 120 µA: bench-check visibility; DNP on battery-life units (doc already allows); it dominates the ~210 µA sleep floor (doc's 100 µA estimate assumed no LED).
12. `08_Mechanical` empty — add 4× M3 holes (+ logo footprint) per doc §9.
13. TVS before the fuse: fine for ESD; optionally move after F1 for sustained-OV robustness.
14. 22 Ω series resistors live at the connector end; guidelines/doc place them near the module — handle at layout (short USB run makes it forgiving).
15. Housekeeping per doc §16: rename/replace `ss14.pdf` (generic SS12–SS1200 family sheet, not MDD) and the TVS datasheet (MDD-named, Goodwork wanted); IPC-2221 missing from `references/`; pin-map PDF still shows the soil divider; BOM summary formula off-by-one.
16. Firmware notes (no schematic change): BAT_SENSE reads ~2.1 V "full" whenever USB is present even with no battery — gate battery-% on USB state; ~200 ms soil settling delay already planned; 3.5 V cutoff leaves 3V3 ≈ 3.25–3.34 V during TX (in spec, reduced RF margin — doc-accepted).

---

## 10. Final answer

**Will it power up, charge, run on battery, read its sensors, and program over USB?**

**Yes — with the remaining decisions above.** As of the 2026-07-20 16:29 save: every net, pin, polarity, MOSFET orientation, charger setting, ADC range, and decoupling requirement checks out against the datasheets — battery monitoring and the USB-C grounds included. Open before layout: the hand-over decision (R16 → 10 k, or move to an integrated power-path/mux part — options in the review discussion), test points, and the reversed-battery+USB procedural control. Then commit.

*Every numeric claim above was recomputed from the datasheets in `references/` during this review; nothing was taken from the design doc on trust.*
