# ESP32-S3 Plant Monitor — Rev A

Wi-Fi plant monitor: soil moisture, temperature / humidity / pressure, and ambient light, USB-C powered with 1-cell LiPo charging and automatic USB↔battery hand-over. Native-USB programming, no bridge chip. KiCad 10 hierarchical schematic, **4-layer board** (signal / GND / GND / signal, JLC04161H-7628), 62.5 × 44.5 mm, ordered fabricated and assembled at JLCPCB on **2026-08-21**.

![ESP32-S3 Plant Monitor Rev A — 3D render](docs/images/board_iso.png)

## Start here

Everything below opens in the browser — no KiCad install, no downloads.

| | |
|---|---|
| 📐 **[Schematic (PDF, 8 sheets)](docs/ESP32S3_PlantMonitor_RevA_Schematic.pdf)** | The whole design, plotted from the board file that was ordered |
| 📄 **[Design document](docs/ESP32S3_Plant_Monitor_Final_Design_Document.md)** | Every component and why it is there, written to be read by someone who is not a PCB engineer |
| 🔍 **[Pre-fab design review](docs/reviews/Design_Review_RevA_2026-07-20.md)** | A real review record with findings, severities and fixes — plus an [independent second pass](docs/reviews/Design_Review_RevA_2026-07-22_Independent.md) |
| 🏭 **[Frozen manufacturing package](fabrication/revA/)** | The exact gerber zip, BOM and CPL uploaded to JLCPCB, beside [`ORDER_NOTES.md`](fabrication/revA/ORDER_NOTES.md) — every setting the board was ordered with |
| 🔧 **[Bring-up guide](docs/BringUp_Guide.md)** | First-power procedure, expected voltages, probe points, troubleshooting |
| 🖨 **[Fab layer plots (PDF)](docs/ESP32S3_PlantMonitor_RevA_FabLayers.pdf)** | All four copper layers, both silks, mask and paste — what the fab actually receives |

| Top | Bottom |
|---|---|
| ![Top](docs/images/board_top.png) | ![Bottom](docs/images/board_bottom.png) |

## What this board demonstrates

- **4-layer discipline** — dual unbroken ground planes, zero signals on the inner layers, a via at every decoupler, stitching around the antenna keep-out.
- **RF module integration** — antenna nose overhanging the board edge, all-layer keepout, ground ring, and a panel-rail instruction written into the fab remark so breakaway tabs cannot sit under the overhang.
- **USB 2.0 pair** at ≈90 Ω coupled geometry, length-matched, no vias, ESD in copper order.
- **Power-path design with numbers** — fuse → TVS → LDO, an automatic USB↔battery hand-over sized so the switch completes in ≈50–100 ms, and a 210 µA sleep budget with a named dominant contributor.
- **Manufacturing fluency** — DFM checked against JLCPCB's live capability pages, part-tier economics understood before ordering, pre-stocked parts library, DDP tariff handling.
- **A documentation trail** — two design reviews, a placement review, a finishing review, an independent final layout audit, and order notes, all on record here.

## Repository map

| Path | What |
|---|---|
| [`hardware/`](hardware/) | KiCad 10 project — root sheet + sheets 02–08, libraries in `hardware/libs/`. Library paths are `${KIPRJMOD}`-relative; keep `libs/` inside `hardware/`. |
| [`fabrication/revA/`](fabrication/revA/) | The frozen order package: gerber zip, BOM, CPL, and `ORDER_NOTES.md` (§7 = the as-ordered record) |
| **Documents** | |
| [`docs/ESP32S3_PlantMonitor_RevA_Schematic.pdf`](docs/ESP32S3_PlantMonitor_RevA_Schematic.pdf) | All 8 schematic sheets, plotted from the ordered board file |
| [`docs/ESP32S3_PlantMonitor_RevA_FabLayers.pdf`](docs/ESP32S3_PlantMonitor_RevA_FabLayers.pdf) | Layer-by-layer fab plots |
| [`docs/ESP32S3_Plant_Monitor_Final_Design_Document.md`](docs/ESP32S3_Plant_Monitor_Final_Design_Document.md) | The full design document — as-built designators throughout; Addendum A holds the renumbering record |
| [`docs/PROJECT_STATUS.md`](docs/PROJECT_STATUS.md) | Live hand-off: current state, waivers on record, standing facts |
| [`docs/BringUp_Guide.md`](docs/BringUp_Guide.md) | First-power procedure, expected voltages, probe points |
| [`docs/PinMap_CheatSheet.md`](docs/PinMap_CheatSheet.md) | GPIO / net / connector map, as built |
| [`docs/Engineering_Notes.md`](docs/Engineering_Notes.md) | Bench notes: calibration values, battery-protection analysis, hand-over walkthrough |
| [`docs/ESP32S3_PlantMonitor_RevA_BOM.md`](docs/ESP32S3_PlantMonitor_RevA_BOM.md) | BOM as a readable table (the [`.xlsx`](docs/ESP32S3_PlantMonitor_RevA_BOM.xlsx) is the editable source) |
| **Layout rulebooks** | |
| [`docs/Hard_Rules_Layout_RevA.md`](docs/Hard_Rules_Layout_RevA.md) | The graded rulebook — LAW vs STRONG PRACTICE, plus 4-layer amendments |
| [`docs/Routing_Guide_RevA_4Layer.md`](docs/Routing_Guide_RevA_4Layer.md) | Layer strategy, net-by-net routing order, via craft |
| [`docs/KiCad_Settings_RevA.md`](docs/KiCad_Settings_RevA.md) | Every board-setup value and the reasoning behind it |
| [`docs/Layout_Readiness_and_Placement_Guide_RevA.md`](docs/Layout_Readiness_and_Placement_Guide_RevA.md) | Footprint verification + placement playbook |
| [`docs/Footprint_Check_ESP32S3_PlantMonitor_RevA.md`](docs/Footprint_Check_ESP32S3_PlantMonitor_RevA.md) | All 20 packages / 69 placements, each checked against its datasheet |
| [`docs/Footprint_Terms_and_Leeway_Guide.md`](docs/Footprint_Terms_and_Leeway_Guide.md) | Plain-English footprint terms and how much tolerance actually matters |
| [`docs/Fabrication_File_Primer.md`](docs/Fabrication_File_Primer.md) | What each gerber and drill file is, and why |
| [`docs/Finishing_Guide_RevA.md`](docs/Finishing_Guide_RevA.md) | Silkscreen, branding and pre-order finishing pass |
| [`docs/RevB_Upgrade_Plan.md`](docs/RevB_Upgrade_Plan.md) | Ranked upgrades, integrated power path first |
| **Reviews** | |
| [`docs/reviews/Design_Review_RevA_2026-07-20.md`](docs/reviews/Design_Review_RevA_2026-07-20.md) | Full pre-fab design review + findings table |
| [`docs/reviews/Design_Review_RevA_2026-07-22_Independent.md`](docs/reviews/Design_Review_RevA_2026-07-22_Independent.md) | Independent second opinion |
| [`docs/reviews/Placement_Review_RevA_2026-08-10.md`](docs/reviews/Placement_Review_RevA_2026-08-10.md) | Placement review (+ [v2 status](docs/reviews/Placement_Review_RevA_2026-08-12_v2_status.md) and annotated maps) |
| [`docs/reviews/Finishing_Review_RevA_2026-08-16.md`](docs/reviews/Finishing_Review_RevA_2026-08-16.md) | Silkscreen / branding / finishing review |
| [`docs/reviews/Final_Layout_Review_RevA_2026-08-17.md`](docs/reviews/Final_Layout_Review_RevA_2026-08-17.md) | Independent final layout audit, immediately pre-order |
| **References** | |
| [`references/datasheets/`](references/datasheets/) | Vendor datasheets for every part — [indexed here](references/datasheets/README.md) |
| [`references/JLCPCB_Capabilities_2026-08.md`](references/JLCPCB_Capabilities_2026-08.md) | Capability quick-sheet used for DFM |
| [`references/reference-designs/`](references/reference-designs/) | Espressif DevKitC-1 schematic |
| [`firmware/`](firmware/) | Firmware project — placeholder; hardware-imposed duties in its README |

## State — 2026-08-21

**Rev A is ordered.** 5 boards, 4-layer, lead-free HASL, Standard PCBA, every component drawn from a pre-stocked JLC Parts Library. DRC at order: **0 errors / 0 unconnected / 0 parity differences** (KiCad 10, zones refilled). The exact uploaded gerber zip, BOM and CPL are frozen in [`fabrication/revA/`](fabrication/revA/) beside the as-ordered record. Reviews are on file through the 2026-08-17 final layout audit.

Next: bring-up per [`docs/BringUp_Guide.md`](docs/BringUp_Guide.md) when the boards land (early September). See [`docs/PROJECT_STATUS.md`](docs/PROJECT_STATUS.md) for the live hand-off.

## Hard rules

1. **Meter the battery plug polarity** before it ever touches J3 — the charge LED is not proof of polarity.
2. **Protected cells only**, and never store the board with a battery attached (~210 µA sleep floor).
3. **Firmware enforces the battery limits**: no Wi-Fi TX below ~3.5 V, deep sleep at 3.0 V.

## License

Hardware design files and documentation are released under the **CERN Open Hardware Licence v2 — Permissive** (see [`LICENSE`](LICENSE)). Vendor datasheets in `references/datasheets/` remain the property of their respective manufacturers and are included for convenience only.
