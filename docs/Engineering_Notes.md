# Engineering Notes — ESP32-S3 Plant Monitor Rev A
*Bench notes and design analyses, organized 2026-07-20 from working notes. Add new entries at the bottom with a date.*

## 1. Soil probe calibration (breadboard, direct connection — matches final schematic)

Measured through the **as-built** input chain (no divider, 100 nF filter, 12 dB attenuation, 12-bit raw):

| Condition | Raw ADC | ≈ Voltage |
|---|---|---|
| Fully wet (in water) | **1465** | ~1.11 V |
| Fully dry (in air) | **2900** | ~2.20 V |

Output falls as moisture rises (inverse relationship). Dry ≈ 2.2 V confirms the divider removal was safe: comfortably inside the ADC's 0–2900 mV effective range (ATTEN=3, ±50 mV per Espressif guidelines). Re-record both constants on the actual PCB at bring-up (step D-11) before trusting absolute readings.

## 2. Battery as received

Pack measured **3.95 V** out of the box (~60–70 % state of charge — normal storage charge; not a fault).

## 3. Breadboard validations to run before/alongside board bring-up

- [ ] Soil probe + 100 k pull-down + MOSFET high-side switch on breadboard: confirm probe output when switched off decays to 0 V (no float), and settling time after power-on (~200 ms budget).
- [ ] Battery-percentage reading on breadboard ESP32: divider ÷2, ×2 in firmware, compare against meter across the pack.
- [ ] Power LED with 10 k resistor: verify it actually lights visibly at ~120 µA with the chosen green LED. If invisible, options: 2.2 k (~550 µA), or keep 10 k and accept "dark = normal."

## 4. Battery protection stack — who guards what

Three layers, from gentle to last-resort. The pack's PCM (Protection Circuit Module, **S-8261AAJMD** per the page-5 pack drawing; S-8261 series: overcharge detect programmable 3.9–4.5 V ±25 mV, overdischarge 2.0–3.0 V, overcurrent-1 at 0.05–0.3 V across the FETs) prevents *disasters*, not wear. The gap between PKCell's operating rules and the PCM's trip points is where a cell gets quietly ruined — something must own each gap:

| Hazard | PKCell's rule | PCM trips at | Who enforces the *rule* on this board |
|---|---|---|---|
| Overcurrent | 500 mA (1 C) max | ~1.5–7.5 A (0.150 V ÷ FET Rds(on)), 9 ms delay | Nobody needed — ESP32 peak is 355 mA; holds by itself |
| Overcharge | 4.20 V | 4.325 V | **MCP73831-2**: stops at 4.20 V ± 0.75 % = 4.232 V worst case → ~93 mV margin below the PCM ✔ |
| Overdischarge | 3.0 V | 2.500 V | **Firmware — nobody else.** Half-volt gap. The design doc delegated this to the PCM and only ever *reported* the level; as-built rule: firmware must act (see below). |

**Firmware thresholds (both read BAT_SENSE / GPIO2; neither is a schematic change):**

* **~3.5 V — stop transmitting.** A *regulator* rule, not a battery rule: the AP2112K needs ~200 mV headroom to hold 3.3 V during a 355 mA Wi-Fi TX peak. Waking, reading sensors, and sleeping remain fine below 3.5 V.
* **3.0 V — deep sleep, stop everything.** The *battery* rule (PKCell Form 1, row 3 discharge floor).

**Deep sleep is not "off."** ~210 µA keeps flowing (power LED ~120, LDO quiescent ~55, sense divider ~21, ESP32 ~10). 500 mAh ÷ 0.21 mA ≈ **100 days** from full to the PCM's 2.5 V disconnect. The PKCell spec lists 0 V-recovery charge as *Unavailable* — **do not store the board with a battery plugged in**; a cell taken to PCM cutoff and left there is scrap.

## 5. Input voltage budget — why the Schottky (SS14), argued in tolls

**The budget.** USB's worst honest voltage: 4.75 V. The regulator's input floor for a clean 3.3 V under TX load: ~3.6 V. Budget = 4.75 − 3.6 = **1.15 V** for every series toll combined.

**The tolls at 355 mA (Wi-Fi TX peak):**

| Element | Toll |
|---|---|
| F1 polyfuse (1206L075/16, worst-case R = 0.29 Ω) | 0.355 A × 0.29 Ω ≈ **0.10 V** |
| One-way door, Schottky SS14 | ≈ **0.40 V** |
| One-way door, ordinary silicon (1N4007-class), for comparison | ≈ 0.90 V |

**The bill.** Schottky: 0.50 V spent, **0.65 V margin left**. Silicon: 1.00 V spent, 0.15 V left — and that 150 mV disappears to any one of: silicon's Vf rising in the cold (board sits by a window), a worse-than-typical diode (spec allows ~1.1 V), or a long/cheap USB cable dropping voltage before the board. The Schottky's margin shrugs all three off. The argument is **margin, not efficiency** — silicon's toll also eats the headroom that keeps Q3's body diode ("the hidden door") firmly shut while USB is present.

## 6. USB-unplug hand-over, moment by moment (the R16 bug and fix)

Track three things: the **+5V_PROT rail** (which is also Q3's gate), **VSYS** (Q3's source), and where current flows. Setup: USB in, battery 3.8 V, rail = 5 V, VSYS ≈ 4.6 V (through D4), Q3 off (gate above source).

1. **Fast drop.** Cable pulled. Nothing refills the rail; the whole board's load drains the rail capacitors fast, and VSYS follows ~0.4 V behind through D4: 5 → 4.5 → 4.0 …
2. **The battery's diode catches VSYS.** At VSYS ≈ 3.15 V (3.8 − 0.65 body-diode toll), Q3's body diode starts feeding the board and VSYS holds there. **At this instant D4 shuts off** — the rail has fallen below VSYS, so rail and VSYS are now separated.
3. **The slow crawl (where the bug lived).** The board no longer drains the rail — only the gate pull-down does. With 100 k the drain is a pinhole: the gate creeps down for **0.5–1 s** while VSYS is stuck at 3.15 V — too low for the LDO to make a clean 3.3 V. Any Wi-Fi burst here ⇒ brown-out reset (~2.98 V).
4. **Q3 switches on.** When the gate crawls ~0.9 V below the source (≈ 2.25 V), the real switch closes, the 0.65 V toll vanishes, VSYS jumps to 3.8 V, everything stabilizes.

**Fix (applied 2026-07-20): R16 100 k → 10 k.** The pinhole becomes a proper drain: crawl shrinks to ~50–100 ms, short enough for the VSYS capacitors and brown-out margin to ride through. Cost: 5 V ÷ 10 k = 0.5 mA extra bleed **only while USB is present** (the node sits at 0 V on battery). This also aligns with the reference design's own guidance (Zak/AN1149: keep the pull-down well under 100 k). The *structural* cure — an integrated power-path IC — is Rev B material: see `RevB_Upgrade_Plan.md`.

**Verify at bring-up:** scope VSYS, unplug at VBAT ≈ 3.7 V, expect a ≤ ~100 ms notch at VBAT − 0.65 V, no reset (BringUp_Guide step 9).

## Sources

* PKCell Li-Polymer 503035 500 mAh datasheet (Form 1; pack drawing p.5) — `references/datasheets/`
* [ABLIC S-8261 series battery-protection IC datasheet](https://www.ablic.com/en/doc/datasheet/battery_protection/S8261_E.pdf)
* MCP73831 datasheet DS20001984H — `references/datasheets/`
* ESP32-S3-WROOM-1 datasheet v1.8 (TX current) + Espressif hardware design guidelines 2026-06-23 (ADC ATTEN=3 range, EN RC) — `references/datasheets/`
* AP2112K datasheet (dropout, Iq) — `references/datasheets/`
* Littelfuse 1206L datasheet (resistance, derating) — `references/datasheets/`
* [Zak Kemble — load sharing](https://blog.zakkemble.net/a-lithium-battery-charger-with-load-sharing/) · [Microchip AN1149](https://ww1.microchip.com/downloads/en/AppNotes/01149c.pdf)

---

## 7. Part selection session (2026-07-20) — decisions & lessons

All 53 parts assigned footprints + LCSC numbers in the schematic (verified 53/53); BOM regenerated from the schematic. Full list lives in `docs/ESP32S3_PlantMonitor_RevA_BOM.xlsx`.

* **LED chemistry rule (learned the hard way):** red/yellow/yellow-green ≈ 2 V forward — fine on a 3.3 V rail through a resistor; blue/white/emerald/pure-green ≈ 3 V — will NOT light from 3.3 V through 10 k. Power LED is KT-0603YG (C2289, Vf 2.0–2.2 V, yellow-green). It's a 30–42 mcd part → ~0.2 mcd at 120 µA: faint indoor glow is *by design*; bench-check per §3.
* **4-pad tactile switch trap:** TS-1187A pads pair A-B (top) / C-D (bottom) internally. Naive 1/2/3/4 numbering put both symbol pins on the same metal — would have held EN at GND forever. Footprint pads renumbered 1/1/2/2 (top/bottom rows), verified against the datasheet circuit diagram. Always check the pairing diagram on 4-leg buttons.
* **Basic vs Extended:** purely "is this exact value permanently loaded in JLC's machines" — not quality. Stock and library status are live (C25804 10 k hit zero stock; C2289 rotated Basic→Extended mid-project). Decisions happen at BOM upload, with alternates noted (10 k alt: C98220).
* **KiCad field truths:** the Fields Table groups by *exact text* (`1u` ≠ `1uF`) **and** by every group-checked column; editor state ≠ disk state until Ctrl-S — all verification (and git, and the fab) reads the disk.
* Values normalized (1uF/4.7uF), D1=SMF5.0A, F1=1206L075/16WR, D2=LED_YG, D3=LED_RED; DNP only on C3/C4.

---
*Add new dated entries below.*
