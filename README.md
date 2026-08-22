# ESP32-S3 Plant Monitor — Rev A

![ESP32-S3 Plant Monitor Rev A — 3D render](docs/images/board_iso.png)

Wi-Fi plant monitor: soil moisture (DFRobot SEN0193), temp/RH/pressure (BME280, 0x76), ambient light (VEML7700, 0x10), USB-C powered with 1-cell LiPo charging (MCP73831, ~100 mA) and automatic USB↔battery hand-over. Native-USB programming, no bridge chip. KiCad 10 hierarchical schematic, **4-layer board (signal / GND / GND / signal, JLC04161H-7628)**, 62.5 × 44.5 mm, JLCPCB assembly.

## Where things live

| Path | What |
|---|---|
| `hardware/` | KiCad project (root sheet + sheets 02–08) with its libraries in `hardware/libs/`. Library paths are `${KIPRJMOD}`-relative — keep `libs/` inside `hardware/`. |
| `docs/` | Living documents — **start here** |
| `docs/BringUp_Guide.md` | First-power procedure, expected voltages, probe points, troubleshooting |
| `docs/PinMap_CheatSheet.md` | GPIO/net/connector map (as built) |
| `docs/Engineering_Notes.md` | Bench notes: calibration values, battery-protection analysis, hand-over walkthrough |
| `docs/RevB_Upgrade_Plan.md` | Ranked upgrades (integrated power path first) |
| `docs/ESP32S3_PlantMonitor_RevA_BOM.xlsx` | **Ordering source of truth** (schematic designators, regenerated 2026-07-20); freeze a copy into `fabrication/revA/` at order time |
| `docs/ESP32S3_Plant_Monitor_Final_Design_Document.docx` | Design doc — see **Addendum A** for as-built deviations & doc↔schematic refdes map |
| `docs/Hard_Rules_Layout_RevA.md` | The graded layout rulebook (LAW / STRONG PRACTICE) + 4-layer amendments |
| `docs/KiCad_Settings_RevA.md` | Every board-setup value and the reasoning behind it |
| `docs/Routing_Guide_RevA_4Layer.md` | **Route from this** — layer strategy, net-by-net order, via craft |
| `docs/Layout_Readiness_and_Placement_Guide_RevA.html` | Footprint verification + placement playbook (2026-08-02) |
| `docs/reviews/` | Review records: design (07-20, 07-22) and placement (08-10, 08-12 + maps) |
| `docs/reviews/Design_Review_RevA_2026-07-20.md` | Full pre-fab design review record + findings |
| `references/datasheets/` | Vendor datasheets (immutable) |
| `references/standards/` | Standards (IPC-2221 to be re-added) |
| `references/reference-designs/` | Espressif DevKitC-1 schematic, app notes |
| `fabrication/` | Factory files, one folder per ordered revision (`revA/` frozen 2026-08-21) |
| `firmware/` | Firmware project (placeholder; hardware-imposed rules in its README) |

## State (2026-08-17)

**See `docs/PROJECT_STATUS.md` for the live hand-off.** **Rev A ordered at JLCPCB 2026-08-21** — 5 boards, 4-layer, lead-free HASL, Standard assembly, every component from the pre-stocked JLC Parts Library. The ordered fab package (gerber zip, BOM, CPL) is frozen in `fabrication/revA/` beside the as-ordered record (`ORDER_NOTES.md` §7). DRC at order: 0 errors / 0 unconnected / 0 parity differences (KiCad 10, zones refilled); reviews on record through the 08-17 final layout review in `docs/reviews/`. Next: bring-up per `docs/BringUp_Guide.md` when the boards land (~early September).

## Hard rules

1. Meter the battery plug polarity before it ever touches J3 (charge LED ≠ proof of polarity).
2. Protected cells only; never store the board with a battery attached (~210 µA sleep floor).
3. Firmware enforces battery limits: no TX below ~3.5 V, shutdown at 3.0 V.

## License

Hardware design files and documentation are released under the **CERN Open Hardware Licence v2 — Permissive** (see `LICENSE`). Vendor datasheets in `references/datasheets/` remain the property of their respective manufacturers.
