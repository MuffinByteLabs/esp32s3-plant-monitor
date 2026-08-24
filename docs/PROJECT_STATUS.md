# Rev A — Order and Build Log
*Updated 2026-08-21 (**Rev A ORDERED**). Board state verified pre-order the same day (file audit + gerber render + JLC placement preview pass). Previous statuses (2026-08-13 board-setup, 2026-08-15 late-stage, 2026-08-18 order-prep) are in git history.*

## Where the project stands

**Rev A IS ORDERED — JLCPCB order placed 2026-08-21: 5 boards, fabricated + Standard-assembled, every component from the JLC Parts Library.** Paid ≈ **$49.26 after coupons** (PCB $12.10 + Standard PCBA $86.79 − ~$49.63 coupons) + **$28.67 DHL Express (DDP)**. Expected timeline: ~3-day fab + Standard assembly + 2–4-day shipping → boards on the bench around the first week of September.

**As ordered** (differences from the 08-17 plan were deliberate, decided on order day):

- PCB: 4-layer, 1.6 mm, green, white silk, FR4 TG135, **LeadFree HASL** (+$5.10), Specify Stackup: No (JLC's standard 4-layer build is the 7628 stackup the board was designed to), outline ±0.2, flying-probe test, min-via tier 0.3 mm/(0.4/0.45).
- **Via covering: Plugged** ($0) — JLC's current 4-layer standard; "Tented" is no longer offered on 4L. Strictly better than the planned tenting, and the gerbers (no mask openings over vias) were already correct for it.
- **Mark on PCB: Remove Mark** (now free) instead of "Specify a location" — no order number prints anywhere; JLC strips the `JLCJLCJLCJLC` token in CAM. The token stays in the design as insurance if this choice ever flips.
- **Assembly: Standard PCBA** — top side, qty 5, tooling holes/rails added by JLCPCB, parts self-service, high-temp (lead-free) paste, nitrogen reflow. The Economic experiment answered itself at BOM matching: **U3 (ESP32 module) AND U4 (BME280) are Standard-only parts.**
- Components line: **$0.00 — all 27 unique parts consumed from My Parts Lib** (the pre-buys paid off in full). Feeder-loading fees $38.25 (13 Extended lines), setup $25.56, stencil $8.21, X-ray $8.20 (module EP inspection), and **J2/J3 THT are hand-soldered by JLC ($3.58 + $0.46)** — which closes ORDER_NOTES §3's open question.
- The antenna/rails **assembly remark was submitted** → shows "quote after review": an engineer reads it before production (expected $0 — it's only a tab-placement request). **If JLC emails a question, answer same-day; production pauses until the reply.**

**Order-day hiccups, all resolved:**

- The five 10 k lines still carried C25804 in the design fields; reassigned to **C98220** in the jlcpcb-tools plugin (which writes the field back) before generating.
- At JLC's matching page, the two 100 k lines (R10/R11 + R14/R15, both C25803) auto-sourced from JLC **global stock showing 0 pcs** — while the plugin's offline DB showed 9.3 M. Re-pointed to **My Parts** (30 owned). Lesson for every future order: **the JLC matching page is the only live stock truth; plugin stock numbers and "group stock" displays both lie** (this is also how the original C25804 pre-order died).
- Placement-preview rotation pass done against a file-derived pin-1/polarity table (all D/Q/U parts; **U1's 45° imported correctly**; BME280's two preview dots = pin 1 at the silk triangle + the lid vent hole, orientation confirmed). Some orientations were corrected **inside JLC's preview**, so the uploaded CPL is pre-correction — **JLC's order preview / their downloadable production files are the rotation truth for Rev A as built.**

## Board changes on 2026-08-21 (included in the ordered gerbers)

The bottom silk is no longer empty: mirrored **ID block** (`ESP32-S3 PLANT MONITOR / Rev A  2026-08 / MuffinByteLabs.com / Designed by Ray Malik`, ~x 62–94 / y 67–77) plus the `JLCJLCJLCJLC` token under J2/J3. **Logo upgraded to `logos:muffinByteLogo_2x`** (2×, 4.4 × 5.8 mm, annotated **G1**, reference hidden in the lib). **PCB title block filled** (title / Rev A / 2026-08-21 / MuffinByteLabs / designer + build-summary comments). **Fab-notes + stackup + revision-history text block on User.Drawings** at (108.5, 51), off-board. SW1/SW2 3D-model paths now `${KIPRJMOD}`-relative on the board. Four of the five trailing-newline silk texts cleaned; the stray silk dot near the antenna edge removed. Consequence recorded in ORDER_NOTES §4: **B.Silkscreen is now a required plot layer.**

Open cosmetic items (zero fab impact; last verified open just before ordering): stackup metadata `copper_finish` still "None" (Board Setup → Physical Stackup → set HAL lead-free) · U5 still lacks the `${REFERENCE}` F.Fab text the other 68 footprints have · one `BME280\n` trailing newline left in a Cmts.User box label · the library master `SW-SMD_4P-L5.1...kicad_mod` still carries an absolute `C:/` 3D path (the board copies are fixed).

## While the boards are in fab

1. **Watch email for the assembly-remark review / DFM questions — reply same-day.**
2. Drop the **order-confirmation PDF** into `fabrication/revA/` (the ordered gerber zip + BOM + CPL were frozen there 2026-08-21).
3. Then **`docs/BringUp_Guide.md` becomes the active document, and its step 1 stands: meter the battery plug before it ever touches J3.**

## Waivers on record

- **TP3 / TP5 / TP9 inside the self-imposed 15 mm antenna ring** (10.6 / 8.3 / 12.5 mm): bare 1.5 mm pads, antenna fully overhangs, vendor guidance has no TP rule — waived for Rev A with probe-lead discipline during RF-active tests (BringUp_Guide); TP5 move noted for Rev B.
- **Rear shield-slot single top spoke:** the slot is tied by an explicit trace and grounded through its barrel into both planes and the bottom pour; reliefs kept so the connector stays hand-replaceable.
- **Single VBUS via at (46.40, 73.95):** capacity margin ≫ load; consistency-only finding.
- **No on-board antenna rule area:** the antenna and its enlarged keep-out sit wholly off-board (module overhangs the top edge) and every zone pulls back 0.5 mm from the edge — there is no on-board region for a rule area to police (also recorded in `KiCad_Settings_RevA.md` §5).

## Settled facts and decisions

Hand-over: R16 = 10 k ⇒ ~50–100 ms body-diode notch at unplug — scope-verify at TP2/VSYS during bring-up (BringUp_Guide step 9) · firmware owns battery limits: no TX < ~3.5 V, shutdown at 3.0 V · sleep floor ~210 µA (power LED dominant; DNP D2 for battery tests) · charge LED ≠ polarity proof — meter the pack plug, always · SW1 = BOOT, SW2 = RESET · **module C2913198 AND BME280 C92489 are Standard-PCBA-tier parts** (confirmed at the 2026-08-21 order) · trace resistance pocket number corrected 2026-08-17: 0.5 mm / 1 oz ≈ **10 mΩ/cm**, not 1 (Hard Rules § pocket numbers) · U1 sits at 45° — verified correct in the Rev A placement preview · JLC reserves extra units on Extended feeder lines for loading attrition (e.g., 15 of 15 D1s, 20 of 20 D2s) — expected, not an error · **DDP shipping = duties prepaid, nothing billed by the courier afterward**; the "My DHL/UPS/FedEx Account" checkout options are for holders of their own courier accounts, not a $2.50 shipping deal · doc↔schematic refdes cross-map lives in the design doc Addendum A.

## Key files

`fabrication/revA/` (**frozen order package**: gerber zip, BOM, CPL + `ORDER_NOTES.md` with the as-ordered record — add the order-confirmation PDF) · `docs/BringUp_Guide.md` (**the active document**; probe table = TP1–TP12) · `docs/Routing_Guide_RevA_4Layer.md` (layer strategy source of truth) · `docs/Hard_Rules_Layout_RevA.md` (+ 4-layer amendments) · `docs/KiCad_Settings_RevA.md` (every setting + why) · `docs/reviews/` (design 07-20/07-22 · placement 08-10/08-12 · finishing 08-16 · final layout 08-17) · `docs/PinMap_CheatSheet.md` (I²C = IO38/IO39, as built) · `references/JLCPCB_Capabilities_2026-08.md` (quick-sheet corrected to 4-layer + order-day tier learnings) · `hardware/` (KiCad; libs inside, `${KIPRJMOD}` paths).
