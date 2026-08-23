# Fabrication packages

Factory output files, one folder per ordered revision. Each folder holds the **exact files uploaded
to the fab** — never a regenerated copy — so what is in here is what was manufactured.

## `revA/` — ordered at JLCPCB 2026-08-21

| File | What |
|---|---|
| `GERBER-ESP32S3_PlantMonitor.zip` | The uploaded gerber set (Excellon drill files inside) |
| `BOM-ESP32S3_PlantMonitor.csv` | The uploaded assembly BOM |
| `CPL-ESP32S3_PlantMonitor.csv` | The uploaded component placement list |
| [`ORDER_NOTES.md`](revA/ORDER_NOTES.md) | Every setting the board was ordered with, the pre-upload gate and how each item closed (§5), and the as-ordered cost and lessons (§7) |

5 boards · 4-layer · lead-free HASL · Standard PCBA · top side only.

**One caveat worth reading before reusing the CPL:** some part rotations were corrected inside
JLCPCB's placement preview *after* CPL upload. The CPL in this folder is therefore pre-correction —
JLC's order preview and their downloadable production files are the rotation truth for Rev A as
built. `ORDER_NOTES.md` §7 lists which parts were touched.

For a browsable view of what these files contain without a gerber viewer, see
[`docs/ESP32S3_PlantMonitor_RevA_FabLayers.pdf`](../docs/ESP32S3_PlantMonitor_RevA_FabLayers.pdf),
and [`docs/Fabrication_File_Primer.md`](../docs/Fabrication_File_Primer.md) for what each file type is.

## Recipe for future revisions

Freeze the exact uploaded zip + BOM + CPL, the order confirmation, and the git tag that produced
them. Tag the commit the fab files were generated from (`revA` marks Rev A) so the package here can
always be traced back to a board file.
