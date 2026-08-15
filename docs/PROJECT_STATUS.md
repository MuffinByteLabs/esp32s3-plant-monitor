# PROJECT STATUS — read me first in a new session
*Updated 2026-08-13, mid 4-layer board-setup session (all statements verified from the saved files). This file is the hand-off between work sessions — update it whenever a milestone lands. Previous status (2026-07-22, pre-layout) is in git history.*

## Where the project stands

**Placement is COMPLETE and twice-reviewed.** Board: **62.5 × 44.5 mm, R4.25 rounded corners with H1–H4 concentric to the arcs** (uniform 1.25 mm edge margin around each M3 screw head). Module re-centered with the antenna fully overhanging the top edge; keep-out empty. All 53 parts + placed TPs on the top side, zero courtyard overlaps, every decoupler measured at its pin (reviews: `docs/reviews/Placement_Review_RevA_2026-08-10.md` + `..._2026-08-12_v2_status.md` with maps). Connector fixes are in: J1 nose overhang **0.96 mm** (HRO target ~1.0), J2/J3 housing faces **+0.30 mm** proud (were 3–3.5 mm off-board).

**USB nets renamed for real diff-pair routing (2026-08-12).** Sheet-02 local labels `USB_CONN_DP/DN` (J1→U1) and `USB_ESD_DP/DN` (U1→R1/R2) replace the auto-generated names; F8 pushed to the board; netclass patterns `*USB_CONN_D*` / `*USB_ESD_D*` added to the `USB` class. The pair now routes with the differential-pair tool end-to-end at 0.25/0.2.

**The board is going to 4 layers: L1 signal · GND1 · GND2 · L4 signal.** Decision note: this **supersedes the 2026-08-06 plan** (recorded in `KiCad_Settings_RevA.md` §5) that had In2 as a +3V3/power-island layer — both inner layers are now solid ground, and power routes as fat traces on the outer layers. Reasons: an uncuttable reference under *both* routing layers, no split-plane crossings to police, and power demand (≤ ~0.5 A) that traces handle easily. Mechanisms and the full routing plan: `docs/Routing_Guide_RevA_4Layer.md`.

**Board Setup state (verified from screenshots + file, 2026-08-13):** Constraints, pre-defined sizes (tracks 0.2/0.25/0.3/0.5/0.8/1.0; vias 0.6/0.3 + 0.8/0.4), and all three net classes with patterns are **correct — done**. Copper count is set to 4 with layers named GND1.Cu/GND2.Cu. **Physical Stackup thicknesses were mis-entered and are being corrected** to JLC04161H-7628 (verify at jlcpcb.com/impedance): F.Cu 0.035 · prepreg 0.2104 (εr 4.4) · GND1 0.0152 · core 1.065 (εr 4.6) · GND2 0.0152 · prepreg 0.2104 · B.Cu 0.035 → readout ≈ 1.6 mm. "Impedance controlled" stays unchecked (order as plain 4-layer; USB is Full Speed).

**Test points:** TP8/TP11/TP12 (UART recovery trio, 2.54 mm pitch) and TP10 (IO0, at 83, 92) are placed. **TP1–TP7 and TP9 are still parked off-board** — verified coordinates for all eight are in the 08-12 v2 status doc §Test-points table (still valid for the current placement).

## Remaining before routing (in order)

1. **Finish Physical Stackup numbers** (table above) — check the bottom readout says ≈1.60 mm.
2. **Place TP1–TP7 + TP9** from the coordinate table.
   - Also fix the one real DRC error the 08-13 external review caught: the THT test-point courtyard is Ø3.0, so the TP12/TP11/TP8 trio at 2.54 mm pitch overlaps (2 courtyard errors at TP11). **Keep the 2.54 pitch** (header compatibility is the point) and shrink the courtyard circle to Ø2.5 in the `TestPoint_THTPad_D2.0mm_Drill1.0mm` footprint (0.25 mm margin around the Ø2.0 pad — same treatment as the earlier SW1/VEML courtyard nits).
3. ~~Decide the SCL/SDA re-pin~~ **DONE 2026-08-15** — SDA → IO38 (pad 31), SCL → IO39 (pad 32), right side facing the sensors; verified from the board netlist after F8 (pads 4/5 released to NC). Firmware: `Wire.begin(38, 39)`. Note: IO39 is MTCK, so pin-JTAG is forfeited — USB-JTAG (the normal S3 path) unaffected. *Routing has since begun (inner GND zones on In1/In2 filled; 300+ segments as of 08-15). Still open from this list: TP courtyard fix if not yet done, remaining TP placement, J1/U3 locks, the 4-layer antenna rule area (zones are now filled, so add it before the outer pours), and the optional TXD0/RXD0/EN/IO0 net labels.*
4. **Lock J1 and U3** (H1–H4 already locked).
5. **Draw the two inner GND zones** (whole board on GND1 and GND2, net `GND`, solid, clearance 0.3 / min width 0.25, pad connection **thermal reliefs**, remove islands) and fill (`B`).
6. **Re-create the antenna keep-out rule area on all four copper layers** — the settings log says one was made 2026-08-06, but the current `.kicad_pcb` contains **no rule area** (checked 2026-08-13); it needs to exist before pours.
7. Final F8 + ERC + **git commit "pre-routing baseline"**.

Then route per `docs/Routing_Guide_RevA_4Layer.md` §3: USB first (the connector breakout now routes entirely on L1 — no crossover via), VBUS chain in copper order, power spine, analog, slow signals, GND vias/pours/stitching, silk pass.

## Late-stage status (2026-08-15)

Routing is **complete** (all pads connected, USB pair via-free and length-matched, power doubled-via'd, every decoupler grounded at its pin). All 12 TPs placed on-copper; THT TP courtyard fixed to Ø2.5 (DRC clean there). **Consciously waived:** the 4-layer antenna rule area, and (pending final call) four TPs inside the 15 mm antenna ring — TP5 (8.0 mm), TP3 (10.4), TP9 (12.0), TP7 (14.0); Espressif's official guidance doesn't address TPs, so this is our own conservative rule — mitigation is probe-lead discipline during RF-active tests. Remaining: follow `docs/Finishing_Guide_RevA.md` (outer pours → stitching incl. R13's via → DRC loop → silk pass → JLC order via the plugin, Standard assembly for the module).

## Standing facts (don't re-derive)

Hand-over: R16 = 10 k ⇒ ~50–100 ms body-diode notch at unplug — scope-verify at TP2/VSYS during bring-up (BringUp_Guide step 9) · firmware owns battery limits: no TX < ~3.5 V, shutdown at 3.0 V · sleep floor ~210 µA (power LED dominant; DNP D2 for battery tests) · charge LED ≠ polarity proof — meter the pack plug, always · SW1 = BOOT, SW2 = RESET (schematic wins over the old design doc) · module C2913198 is Standard-PCBA tier · C3/C4 carry `dnp` but not exclude-from-BOM — confirm both stay off BOM/placement at JLC upload · doc↔schematic refdes cross-map lives in the design doc Addendum A.

## Key files

`docs/Routing_Guide_RevA_4Layer.md` (route from this) · `docs/Hard_Rules_Layout_RevA.md` (+ 4-layer amendments at the end) · `docs/KiCad_Settings_RevA.md` (every setting + why) · `docs/reviews/` (placement reviews 08-10, 08-12 with maps; design reviews 07-20, 07-22) · `docs/BringUp_Guide.md` (probe table = TP1–TP12) · `docs/PinMap_CheatSheet.md` · `hardware/` (KiCad; libs inside, `${KIPRJMOD}` paths) · `references/datasheets/`.
