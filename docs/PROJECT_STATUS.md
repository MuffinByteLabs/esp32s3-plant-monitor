# PROJECT STATUS — read me first in a new session
*Updated 2026-07-22, end of the test-point & mounting-hole session (all changes netlist-verified from the saved files; ERC clean). This file is the hand-off between work sessions — update it whenever a milestone lands.*

## Where the project stands

Schematic is **electrically complete and reviewed** (full pre-fab review: `docs/reviews/Design_Review_RevA_2026-07-20.md`). All fixes applied and netlist-verified: USB-C GND pins grounded, BAT_SENSE chain in, R16 = 10 k hand-over fix, J2 pin order matches probe.

**Part selection is DONE — all 53 parts have footprint + LCSC number + correct DNP (verified in-file 53/53).** Highlights: switch = XKB TS-1187A (C318884) with imported footprint, pads renumbered 1/1/2/2 to match its A-B/C-D internal pairing (naive numbering would have shorted EN to GND permanently); green LED = KT-0603YG C2289 (yellow-green Vf ~2.1 V — emerald/pure greens are 3 V chemistry and won't light from 3.3 V through 10 k); 10 k resistor C25804 had zero stock on selection day — alternate YAGEO C98220 noted in BOM. BOM regenerated **from the schematic** (`docs/ESP32S3_PlantMonitor_RevA_BOM.xlsx`, 29 lines / 51 placed + 2 DNP); library-status and stock are live values — re-verify at order upload.

**Test points & mounting holes are IN (2026-07-22; every TP netlist-verified):** TP1 +5V_PROT · TP2 VSYS · TP3 +3V3 · TP4 VBAT · TP5 BAT_SENSE · TP6 ADC_SOIL · TP7+TP8 GND · TP9 EN · TP10 IO0 · TP11 TXD0 · TP12 RXD0 (TP9–TP12 wired directly on sheet 04 — local nets, no labels). **Deviation from the all-flat plan:** TP8/TP11/TP12 use `TestPoint_THTPad_D2.0mm_Drill1.0mm` through-hole pads (header pin / DuPont jumper fits) — the UART recovery trio: adapter TXD→TP12, RXD→TP11, GND→TP8; hold SW1=BOOT, tap SW2=RESET; 3.3 V logic, never an adapter's power pins. The other nine are flat `TestPoint_Pad_D1.5mm`; all 12 Exclude-from-BOM (BOM unchanged: 29 lines / 53 parts stands). Sheet-07 `VBAT` label promoted local→global so the sheet-08 TP reaches it; TXD0/RXD0 no-connect flags removed. H1–H4 = `MountingHole_3.2mm_M3`, no pins.

**JLCPCB plugin (kicad-jlcpcb-tools) is installed** in KiCad; its parts DB downloads on first open in the PCB editor. PCB layout NOT started (`.kicad_pcb` is a stub). Git: everything through the folder reorganization is committed (`96af1f8`) and pushed; **all part-selection work since is UNCOMMITTED** — commit after the remaining schematic steps below.

## Remaining before layout (in order)

1. **Push the milestone commit** ("pre-layout: footprints, LCSC fields, test points, mounting holes", made 2026-07-22). The sandbox that made it has no network — run `git push` from a normal terminal.
2. **Board setup decisions (10-min discussion, then F8):** rough board size + which edges get USB-C / battery / probe connectors; antenna plan (module nose overhangs board edge, full copper keep-out); PCB design rules for JLC 2-layer — defaults 0.2 mm clearance/track, signals 0.25 mm, power nets ≥ 0.5 mm (doc §9), vias 0.6/0.3 mm. Then **Update PCB from Schematic (F8)** and layout begins.

Done 2026-07-22: 12 test points (verified per-net), H1–H4, ERC clean, fp-lib-table de-duplicated, BringUp/PinMap probe tables rewritten to real TPs.

## Layout-phase placement rules (from design doc §9 + review)

Protection parts (F1, D1, U1) immediately at the USB connector · module antenna past the board edge, no copper/silk under it, stitching vias around (never inside) the keep-out · LDO U2 gets generous copper (~0.26 W worst) · BME280 far from LDO/module heat, vent hole exposed · VEML7700 clear sky view, away from LEDs · ADC traces (ADC_SOIL, BAT_SENSE) short, away from USB pair and antenna · USB D+/D− short, coupled, via-free over solid ground · battery polarity marks on J3 silkscreen · BOOT/RESET silk labels: **SW1 = BOOT, SW2 = RESET** (schematic refs; design doc has them swapped — schematic wins) · cluster the through-hole recovery trio **TP11/TP12/TP8** together near a board edge; label all TPs on silk.

## Standing facts (don't re-derive)

Hand-over: R16 = 10 k ⇒ ~50–100 ms body-diode notch at unplug — scope-verify at bring-up (BringUp_Guide step 9) · firmware owns battery limits: no TX < ~3.5 V, shutdown at 3.0 V · sleep floor ~210 µA (power LED dominant; DNP D2 for battery tests) · charge LED ≠ polarity proof — meter the pack plug, always · module C2913198 is Standard-PCBA tier · doc↔schematic refdes cross-map lives in the design doc **Addendum A**.

## Key files

`docs/BringUp_Guide.md` (probe table = TP1–TP12, updated 2026-07-22) · `docs/PinMap_CheatSheet.md` · `docs/Engineering_Notes.md` (bench data & analyses) · `docs/RevB_Upgrade_Plan.md` · `docs/reviews/Design_Review_RevA_2026-07-20.md` · `hardware/` (KiCad; libs inside, `${KIPRJMOD}` paths) · `references/datasheets/`.
