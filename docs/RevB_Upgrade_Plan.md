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

## 2. Do these in the Rev A *layout* (free wins, not really Rev B)

* **Test points TP1–TP15** per design doc §11 — the schematic currently has none. Minimum set: +5V_PROT, VSYS, +3V3, GND ×2, VBAT, BAT_SENSE, ADC_SOIL, EN, IO0, TXD0, RXD0.
* **Mounting holes** (4× M3) — `08_Mechanical` sheet is still empty.
* Silkscreen: **battery polarity marks at J3**, probe pin names at J2, BOOT/RESET labels matching actual refs (SW1 = BOOT, SW2 = RESET).
* UART fallback: TXD0/RXD0 at least on pads (recovery path if native USB is ever bricked).

## 3. Sleep-floor reductions (months → half-year battery life)

Current floor ≈ 210 µA: power LED 120 + LDO Iq 55 + divider 21 + ESP32 ~10.

* **Power LED on a solder jumper** (or DNP by default): −120 µA.
* **Low-Iq LDO**: TI TPS7A02 (25 nA Iq, 200 mA — check peak headroom vs. Wi-Fi TX; may need output bulk) or HT7833 / XC6220 class (~1–8 µA Iq, 300–500 mA): −47…−55 µA.
* **Switched battery divider**: high-side P-FET on the divider driven by a GPIO (sample-then-off), or 1 MΩ + 1 MΩ with a small buffer: −20 µA.
* Achievable floor ≈ 15–35 µA ⇒ 500 mAh ≈ 1.5–3 years of pure sleep; wake cycles then dominate.

## 4. Power robustness

* **Buck-boost regulator** (TPS63001/TPS63020 class) instead of LDO: full 3.0–4.2 V battery range usable at 3.3 V, no TX-headroom rule, ends the 3.5 V firmware cutoff. Cost: inductor, EMI care near the ADCs/antenna.
* **TVS after the fuse** (or a second one) so sustained overvoltage trips F1 instead of cooking D1.
* **ESD/robustness on field wiring**: series R + TVS on ADC_SOIL at J2 (the probe cable is an antenna), and consider the same on J3.
* **Charger with pack-NTC support** (comes free with BQ24075's TS pin) if a thermistor-equipped pack is adopted.

## 5. Usability / debug

* Qwiic/STEMMA-QT (JST-SH 4-pin) connector on the I²C bus for external sensors.
* Charge-state readable by firmware (STAT to a GPIO through a divider — mind the 5 V domain, see Zak's app-note variant) so the app can report "charging/full."
* RGB status LED with firmware duty-cycling instead of the always-on power LED.
* Label probe points / add a scope-ground pad near VSYS.

## 6. Process

* Reconcile reference designators doc ↔ schematic once, then regenerate BOM/pin-map from the schematic only (single source of truth).
* Add `kicad-cli sch erc` / `pcb drc` to a pre-commit or CI script.
* Keep `docs/Engineering_Notes.md` updated per bench session; fold conclusions into the design doc addendum at each rev.

## Sources

* [Microchip AN1149 — Li-Ion charger with load sharing](https://ww1.microchip.com/downloads/en/AppNotes/01149c.pdf)
* [TI BQ24075 product page](https://www.ti.com/product/BQ24075) · [datasheet PDF](https://cdn.sparkfun.com/assets/learn_tutorials/5/3/0/bq24075.pdf)
* [Microchip MCP73871 product page](https://www.microchip.com/en-us/product/mcp73871) · [datasheet PDF](https://ww1.microchip.com/downloads/en/DeviceDoc/MCP73871-Data-Sheet-20002090E.pdf)
* [TI TPS2113A product page](https://www.ti.com/product/TPS2113A) · [datasheet](https://www.ti.com/lit/gpn/TPS2113A)
* [Zak Kemble — load sharing article](https://blog.zakkemble.net/a-lithium-battery-charger-with-load-sharing/)
