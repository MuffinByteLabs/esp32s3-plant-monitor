# Design Reviews — ESP32-S3 Plant Monitor Rev A

Six reviews on file, running from schematic capture to the last check before the order went to JLCPCB. Each one is a real record: what was examined, how, what was found, and what changed as a result. Nothing here is a summary written after the fact.

| Date | Review | Method | Outcome |
|---|---|---|---|
| 2026-07-20 | [Pre-fabrication design review](Design_Review_RevA_2026-07-20.md) | Netlist parsed from the `.kicad_sch` sources to pin level, cross-checked against every datasheet in `references/` | **3 blocking findings**, all resolved the same day — see below |
| 2026-07-22 | [Pre-layout review: netlist re-extraction](Design_Review_RevA_2026-07-22_Netlist_Reextraction.md) | Netlist rebuilt from source a second time, independently of the first pass, plus a full datasheet cross-check | No blocking or functional findings. Cleared for layout |
| 2026-08-10 | [Placement review](Placement_Review_RevA_2026-08-10.md) | Component-by-component against the placement rulebook, with an [annotated map](Placement_Review_RevA_2026-08-10_map.png) | Outline shrink, module re-centring and connector clearances; [v2 status](Placement_Review_RevA_2026-08-12_v2_status.md) confirms each item landed |
| 2026-08-12 | [Placement review v2 status](Placement_Review_RevA_2026-08-12_v2_status.md) | Re-check of every 08-10 item against the revised board, with an [annotated map](Placement_Review_RevA_2026-08-12_v2_map.png) | All items closed; outline down to 62.5 × 44.5 mm with R4.25 corners |
| 2026-08-16 | [Finishing review](Finishing_Review_RevA_2026-08-16.md) | Scripted geometry pass over the saved board file — silkscreen, branding, zone fills, LCSC coverage | Silkscreen and DNP/BOM-exclusion fixes before the final audit |
| 2026-08-17 | [Final layout audit](Final_Layout_Review_RevA_2026-08-17.md) | KiCad 10.0.5 headless DRC with zone refill and schematic parity, plus an s-expression geometry audit of every rule in the rulebook | **0 errors · 0 unconnected · 0 parity differences.** Cleared to order |

## The three blocking findings, 2026-07-20

These are the reason the review process exists, and all three were caught before any money was spent:

1. **USB-C ground pins were no-connected.** J1's A1/B1/A12/B12 carried an explicit no-connect flag and touched no copper — ERC was clean precisely *because* the flag told it the omission was intentional. Those four pins are the DC return for every milliamp of VBUS current and the reference for D+/D−; the shield is a separate EMI screen, not a supply return. Symptoms would have ranged from "works with cable A, dead with cable B" to a defeated ESD strategy. Fixed and re-verified the same afternoon.
2. **The saved files were missing the battery-sense chain.** Battery monitoring existed in the design document but not in the committed schematic — the board would have shipped with no way to read its own battery voltage.
3. **The BOM was stale.** Designators no longer matched the schematic, which for an assembled order means parts placed at the wrong references.

A fourth finding, ranked RISKY rather than blocking, changed R16 from 100 kΩ to 10 kΩ: at 100 kΩ the USB-unplug hand-over took 0.5–1 s of body-diode sag, long enough to brown out the MCU on every unplug at mid-battery. At 10 kΩ it completes in ≈50–100 ms.
