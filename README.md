# ESP32-S3 Plant Monitor — Rev A

Wi-Fi plant monitor: soil moisture (DFRobot SEN0193), temp/RH/pressure (BME280, 0x76), ambient light (VEML7700, 0x10), USB-C powered with 1-cell LiPo charging (MCP73831, ~100 mA) and automatic USB↔battery hand-over. Native-USB programming, no bridge chip. KiCad 10 hierarchical schematic, 2-layer board, JLCPCB assembly.

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
| `docs/reviews/Design_Review_RevA_2026-07-20.md` | Full pre-fab design review record + findings |
| `references/datasheets/` | Vendor datasheets (immutable) |
| `references/standards/` | Standards (IPC-2221 to be re-added) |
| `references/reference-designs/` | Espressif DevKitC-1 schematic, app notes |
| `fabrication/` | Factory files, one folder per ordered revision (empty until first order) |
| `firmware/` | Firmware project (placeholder; hardware-imposed rules in its README) |

## State (2026-07-20)

Schematic reviewed, netlist-verified, and corrected: USB-C GND pins grounded, battery-sense chain in place, hand-over pull-down = 10 k. Open before layout: add test points TP1–TP15 and mounting holes (sheet 08), then commit. Layout not started (`.kicad_pcb` is a stub).

## Hard rules

1. Meter the battery plug polarity before it ever touches J3 (charge LED ≠ proof of polarity).
2. Protected cells only; never store the board with a battery attached (~210 µA sleep floor).
3. Firmware enforces battery limits: no TX below ~3.5 V, shutdown at 3.0 V.
