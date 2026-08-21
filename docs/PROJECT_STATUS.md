# PROJECT STATUS — read me first in a new session
*Updated 2026-08-18 (parts ordered); board state verified 2026-08-17, pre-order (all statements verified from the saved files: board DRC re-run with zones refilled + schematic parity on). This file is the hand-off between work sessions — update it whenever a milestone lands. Previous statuses (2026-08-13 board-setup, 2026-08-15 late-stage) are in git history.*

## Where the project stands

**Layout is COMPLETE — routing, pours, stitching, and the silk pass are done; the board is at the order-prep stage.** Board: **62.5 × 44.5 mm, R4.25 rounded corners, H1–H4 concentric to the arcs**, 4-layer **L1 signal · GND1 · GND2 · L4 signal** (JLC04161H-7628, stackup numbers entered, readout 1.6062 mm). 416 track segments, 157 vias (only the two sanctioned sizes), zero tracks on either inner plane.

**Verified clean as of 2026-08-17 (KiCad 10 DRC, zones refilled, parity on):** 0 errors · 0 unconnected items · 0 schematic-parity differences. The only warnings are the 15 expected silk-clipped-at-edge items from the overhanging J1/J2/J3/U3 footprint graphics. Highlights confirmed from the file:

- **USB:** D+/D− entirely on F.Cu, zero vias, 0.29/0.2 coupled geometry (~90 Ω on this stackup), total-length mismatch 0.42 mm, ESD chip in copper order, connector breakout all-L1 per Routing Guide §3.1.
- **Power:** every power segment 0.5 mm; layer hops doubled 0.8/0.4 (one conscious single VBUS via at (46.40, 73.95) — see waivers); VSYS all-top compact star; +3V3 flows C9 → C10 → module pin 2.
- **Decoupling:** all 16 fitted caps have their GND via 1.1–1.45 mm from the pad; C14/C17 at the ADC pins.
- **J1 grounding (fixed 2026-08-17):** the four G pads (A1/B12, A12/B1) are **solid** to the top pour with a GND via ~1.1 mm from each pad pair; shield slots stay on thermal reliefs (they ground through their barrels on all four layers, and stay hand-reworkable).
- **Thermal:** U2/U6/U4 ground pads solid; U2 has 5 GND vias within 3 mm; all THT (J2/J3/TPs) on thermal reliefs — hand-solderable.
- **Exclusions:** C3/C4 dnp + excluded from BOM/CPL **on both schematic and board** (parity now clean); all 12 TPs + H1–H4 excluded from BOM and position files.

## Parts status (2026-08-18)

All in-stock lines for 5 boards + attrition are **purchased into the JLC Parts Library** (~$82). **AP2112K-3.3 (C51118) pre-order paid — awaiting JLC's quotation (48 h window from payment); this is the only schedule gate.** The 10 k pre-order (C25804) **failed** — JLC could not obtain a supplier quotation despite large displayed group stock — and was auto-refunded ($8.61); **the 10 k is now C98220** (Yageo RC0603FR-0710KL, Extended, in stock; BOM + ORDER_NOTES updated). At assembly BOM matching, select C98220 for R5/R6/R7/R13/R16. Fallback if the LDO pre-order also fails: DNP U2 at assembly, hand-solder from DigiKey at bring-up.

## Remaining before export (minutes, not hours)

1. **Two antenna-fence vias** — the top-edge GND row still has its two over-target gaps: 4.62 mm (x 53.9 → 58.5) and 5.73 mm (x 72.1 → 77.8). Drop one via near **(56, 51.2)** and one near **(75.5, 51.0)** (the second lands on the existing 0.8 mm GND trace from U3 pad 40). Target is the ~3 mm (λ/20) fence pitch in front of the antenna.
2. Optional, 30 seconds each: double the single VBUS via at (46.40, 73.95) or waive it (0.4 mm drill carries several × the load); clean the trailing newlines inside the `5V` / `EN` / `+` silk texts; add a board-name/rev text and a `JLCJLCJLCJLC` text where the order number should print (without it JLC picks the spot).
3. **Refill (`B`) → final DRC → git commit "pre-order baseline".**
4. Generate fab files and order per **`fabrication/revA/ORDER_NOTES.md`** (settings, remark text, preview checklist — new 2026-08-17).

## Waivers on record (each one sentence, per the house rule)

- **TP3 / TP5 / TP9 inside the self-imposed 15 mm antenna ring** (10.6 / 8.3 / 12.5 mm): bare 1.5 mm pads, antenna fully overhangs, vendor guidance has no TP rule — waived for Rev A with probe-lead discipline during RF-active tests (BringUp_Guide); TP5 move noted for Rev B.
- **Rear shield-slot single top spoke:** the slot is tied by an explicit trace and grounded through its barrel into both planes and the bottom pour; reliefs kept so the connector stays hand-replaceable.
- **Single VBUS via at (46.40, 73.95)** if not doubled: capacity margin ≫ load; consistency-only finding.
- **No on-board antenna rule area:** the antenna and its enlarged keep-out sit wholly off-board (module overhangs the top edge) and every zone pulls back 0.5 mm from the edge — there is no on-board region for a rule area to police (also recorded in `KiCad_Settings_RevA.md` §5).

## Standing facts (don't re-derive)

Hand-over: R16 = 10 k ⇒ ~50–100 ms body-diode notch at unplug — scope-verify at TP2/VSYS during bring-up (BringUp_Guide step 9) · firmware owns battery limits: no TX < ~3.5 V, shutdown at 3.0 V · sleep floor ~210 µA (power LED dominant; DNP D2 for battery tests) · charge LED ≠ polarity proof — meter the pack plug, always · SW1 = BOOT, SW2 = RESET (schematic wins over the old design doc) · module C2913198 is Standard-PCBA tier · trace resistance pocket number corrected 2026-08-17: 0.5 mm / 1 oz ≈ **10 mΩ/cm**, not 1 (Hard Rules § pocket numbers) · U1 sits at 45° — legal everywhere, but double-check its rotation in the JLC placement preview (importers mangle odd angles first) · doc↔schematic refdes cross-map lives in the design doc Addendum A.

## Key files

`fabrication/revA/ORDER_NOTES.md` (order settings + remark + preview checklist — **use this at upload**) · `docs/Routing_Guide_RevA_4Layer.md` (layer strategy source of truth) · `docs/Hard_Rules_Layout_RevA.md` (+ 4-layer amendments) · `docs/KiCad_Settings_RevA.md` (every setting + why; USB DP width updated to as-built 0.29) · `docs/reviews/` (design 07-20/07-22 · placement 08-10/08-12 · finishing 08-16 · final layout 08-17) · `docs/BringUp_Guide.md` (probe table = TP1–TP12) · `docs/PinMap_CheatSheet.md` (I²C = IO38/IO39, as built) · `hardware/` (KiCad; libs inside, `${KIPRJMOD}` paths) · `references/datasheets/`.
