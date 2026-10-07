# Rev B Upgrade Plan — ESP32-S3 Plant Monitor
*Living document, started 2026-07-20. Items are ranked by value-per-effort; nothing here blocks Rev A.*

## 1. Integrated power path (the headline upgrade)

Rev A uses the app-note discrete load-sharing circuit (MCP73831 + SS14 + P-FET + pull-down, per Microchip AN1149 / Zak Kemble). With R16 = 10 k it works, but it inherently has a ~50–100 ms body-diode phase (0.65 V sag) at USB unplug, a Vgs-threshold-dependent switch point, and no charge safety timer. Rev B should replace the four-part cluster with one IC:

| Option | What it is | Replaces | Pros | Cons |
|---|---|---|---|---|
| **TI BQ24075** (QFN-16) | 1.5 A charger **+ power path** (DPPM) | MCP73831, D4, Q3, R16 | Seamless autonomous switchover; system OUT separate from BAT; **safety timer**; thermal regulation; TS pin for pack NTC; huge design-in base | New part to learn; QFN; TS needs a resistor if pack has no thermistor |
| **Microchip MCP73871-2CC** (QFN-20) | "System load sharing + charge management," same family as Rev A charger | MCP73831, D4, Q3, R16 | Same-vendor continuity; 4.20 V option; ideal-diode battery path; VPCC gives system priority over charge current; USB current-limit pins | QFN-20; more pins to strap |
| **TI TPS2113A** (MSOP-8) power mux, *keep* MCP73831 | Autoswitching 2-input mux, internal FETs | D4, Q3, R16 only | Smallest schematic change; hand-solderable; no diode drop (VSYS ≈ 5 V on USB); reverse-blocking both ways; µs switchover | Charger stays timer-less; two ICs instead of one |

Recommendation: **BQ24075** if redoing the block properly; **TPS2113A** if minimizing change. Verify JLCPCB stock/price for all three before choosing.

## 2. Do these in the Rev A *layout* (free wins, not really Rev B) — *status 2026-08-13*

* ~~**Test points TP1–TP15**~~ **DONE 2026-07-22** — TP1–TP12 in the schematic, netlist-verified; placement finishing now.
* ~~**Mounting holes** (4× M3)~~ **DONE** — H1–H4, now concentric with the R4.25 board corners.
* ~~Silkscreen: **battery polarity marks at J3**, probe pin names at J2, BOOT/RESET labels (SW1 = BOOT, SW2 = RESET)~~ **DONE 2026-08-16/17** — full functional silk pass (TP names, +/− at J3, SIG/PWR/GND at J2, BOOT/RESET, PWR/CHG).
* ~~UART fallback~~ **DONE** — TP11/TP12/TP8 through-hole recovery trio at the bottom edge, 2.54 mm pitch.

## 3. Sleep-floor reductions (months → half-year battery life)

Current floor ≈ 210 µA: power LED 120 + LDO Iq 55 + divider 21 + ESP32 ~10.

* **Power LED on a solder jumper** (or DNP by default): −120 µA.
* **Low-Iq LDO**: TI TPS7A02 (25 nA Iq, 200 mA — check peak headroom vs. Wi-Fi TX; may need output bulk) or HT7833 / XC6220 class (~1–8 µA Iq, 300–500 mA): −47…−55 µA.
* **Switched battery divider**: high-side P-FET on the divider driven by a GPIO (sample-then-off), or 1 MΩ + 1 MΩ with a small buffer: −20 µA. **This also restores U6's battery-detection circuit — see the note below.**
* Achievable floor ≈ 15–35 µA ⇒ 500 mAh ≈ 1.5–3 years of pure sleep; wake cycles then dominate.

> ### Note added 2026-09-09 — the BAT_SENSE divider defeats U6's battery detection
>
> Found during Rev A bring-up, confirmed against the MCP73831 datasheet (DS20001984H §4.2)
> and the part's own option codes. **This corrects an earlier claim in `BringUp_Guide.md` §5
> Observation 2, which states the MCP73831 "has no battery-detection circuit at all." It has one.
> Our divider stops it working.**
>
> **The mechanism.** U6 sources a **6 µA** probe current (I_BAT_DET, 0.6 µA min) out of the VBAT
> pin. If VBAT rises to **V_REG + 100 mV = 4.30 V** under that probe, the charger concludes no
> battery is present. The datasheet is explicit that this requires the node impedance before the
> pack is connected to be **greater than 7 MΩ**.
>
> **Rev A has R14 + R15 = 200 kΩ permanently across VBAT — 35× lower than required.** The divider
> sinks ~21 µA at 4.2 V where the detector sources only 6 µA, so the probe can never pull the node
> up to 4.30 V. U6 therefore reports "battery present" unconditionally, pack or no pack.
>
> **Fitted part is MCP73831T-2ACI/OT — options `AC`**, which sets the thresholds that follow:
>
> | Parameter | Ratio (option AC) | Value at V_REG = 4.20 V |
> |---|---|---|
> | I_PREG / I_REG (precondition current) | 10 % | 10 mA |
> | V_PTH / V_REG (precondition threshold) | 66.5 % | 2.79 V |
> | I_TERM / I_REG (termination current) | 7.5 % | 7.5 mA |
> | **V_RTH / V_REG (auto-recharge threshold)** | **96.5 %** | **4.05 V** |
>
> **Observable consequence, no pack fitted.** VBAT is not a DC level — it is a sawtooth. C16
> (4.7 µF) charges to 4.20 V in ~7 µs, current collapses far below the 7.5 mA termination
> threshold, U6 latches off, and the 200 kΩ divider then drains C16 through the **147 mV**
> hysteresis window (4.20 → 4.05 V) in **~34 ms**, whereupon auto-recharge restarts it. That is a
> **~30 Hz** cycle. STAT is held low only for the refill plus the termination comparator's
> t_TERM filter (~1.3 ms), giving D3 a **~4 % duty cycle** — dim, and above the flicker-fusion
> rate, so it reads as steady rather than blinking. This is the quantitative version of the
> D3 brightness table in `BringUp_Guide.md` §5 Observation 2.
>
> **Why it matters beyond cosmetics.** The same P-FET that saves the 21 µA in the bullet above
> raises the node impedance above 7 MΩ while the divider is parked, so battery detection starts
> working — U6 could then distinguish "no pack" from "flat pack." One part, three wins: sleep
> current, working detection, and a VBAT reading that stops lying at ~4.13 V mean when no cell
> is fitted.
>
> **Caveat for the layout:** with the divider switched off, VBAT's only remaining DC path is
> U6 itself. Confirm nothing else (leakage, scope probe, TP4 fixture) drags the node below
> 7 MΩ, or detection still will not work.

## 4. Power robustness

* **Buck-boost regulator** (TPS63001/TPS63020 class) instead of LDO: full 3.0–4.2 V battery range usable at 3.3 V, no TX-headroom rule, ends the 3.5 V firmware cutoff. Cost: inductor, EMI care near the ADCs/antenna.
* **TVS after the fuse** (or a second one) so sustained overvoltage trips F1 instead of cooking D1.
* **ESD/robustness on field wiring**: series R + TVS on ADC_SOIL at J2 (the probe cable is an antenna), and consider the same on J3. *(2026-08-08 note in Hard Rules: as drawn, ADC_SOIL has no series element on-board — R11 is a parallel pull-down — so a small series R here is confirmed as the Rev B item.)*
* **Thermal slots around the BME280** — only if Rev A bring-up shows a board-heat reading offset beyond simple firmware correction. The 4-layer ground planes spread heat board-wide (see `Routing_Guide_RevA_4Layer.md` §5); characterize first, slot second.
* **Charger with pack-NTC support** (comes free with BQ24075's TS pin) if a thermistor-equipped pack is adopted.

## 5. Usability / debug

* Qwiic/STEMMA-QT (JST-SH 4-pin) connector on the I²C bus for external sensors.
* Charge-state readable by firmware (STAT to a GPIO through a divider — mind the 5 V domain, see Zak's app-note variant) so the app can report "charging/full."
* RGB status LED with firmware duty-cycling instead of the always-on power LED.
* Label probe points / add a scope-ground pad near VSYS.
* Move TP5 (BAT_SNS) a few mm further from the antenna (8.3 mm in Rev A — waived with probe-lead discipline; free to fix in a re-layout).
* One extra GND via tight against U6's ground pad (Rev A has 2 within 3 mm; the LDO got 5 — match the treatment).

## 6. Process

* ~~Reconcile reference designators doc ↔ schematic once~~ — **done 2026-08-23**: the design document body was renumbered to as-built designators and the renumbering record moved into its Addendum A. Keep regenerating BOM/pin-map from the schematic only.
* Add `kicad-cli sch erc` / `pcb drc` to a pre-commit or CI script.
* Keep `docs/Engineering_Notes.md` updated per bench session; fold conclusions into the design doc addendum at each rev.

## Sources

* [Microchip AN1149 — Li-Ion charger with load sharing](https://ww1.microchip.com/downloads/en/AppNotes/01149c.pdf)
* MCP73831/2 datasheet DS20001984H §1.0 (Electrical Characteristics), §4.2 (Battery Detection), §4.6–4.8 (CV mode, termination, auto-recharge), and the Product Identification System option table — local copy at `references/datasheets/CHARGER_Microchip_MCP73831_LiPo_Linear_Charger_Datasheet_DS20001984H.pdf`
* [TI BQ24075 product page](https://www.ti.com/product/BQ24075) · [datasheet PDF](https://cdn.sparkfun.com/assets/learn_tutorials/5/3/0/bq24075.pdf)
* [Microchip MCP73871 product page](https://www.microchip.com/en-us/product/mcp73871) · [datasheet PDF](https://ww1.microchip.com/downloads/en/DeviceDoc/MCP73871-Data-Sheet-20002090E.pdf)
* [TI TPS2113A product page](https://www.ti.com/product/TPS2113A) · [datasheet](https://www.ti.com/lit/gpn/TPS2113A)
* [Zak Kemble — load sharing article](https://blog.zakkemble.net/a-lithium-battery-charger-with-load-sharing/)
