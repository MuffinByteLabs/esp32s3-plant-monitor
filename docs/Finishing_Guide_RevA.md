# Finishing Guide — Rev A: Pours, Stitching, DRC, Silk, Order
*Written 2026-08-15 for the final stretch, with exact KiCad 10 click paths. Board state at writing: routing complete, all 12 TPs placed on-copper, TP courtyards fixed, inner planes filled. Open decisions carried in: the four TPs inside the 15 mm antenna ring (my call — waive with lead discipline, or move TP9/TP3 cheaply), three single-via +3V3 hops (optional doubling), R13 ground via (do it during the stitching pass below).*

---

## Phase 1 — Pour ground on the outer layers (~10 min)

The two inner planes already exist. Now the top and bottom get ground poured into every unused space.

1. Right toolbar → **Add Filled Zone** (or Place → Zone). Click once at a corner *outside* the board outline — the dialog opens before drawing.
2. In the dialog: **Layer: F.Cu** · **Net: GND** · Clearance **0.3** · Minimum width **0.25** · Pad connections: **Thermal reliefs** · Remove islands: **Below area limit** · Fill: solid. OK.
3. Draw a rectangle that completely encloses the board (corners out in the dark area — KiCad clips the fill to my outline minus the 0.5 mm edge rule automatically). Double-click to close it.
4. Repeat for **B.Cu** (same settings). Tip: in the layer list of the zone dialog I can tick F.Cu and B.Cu together and draw once.
5. Press **`B`** (Fill All Zones). The board floods; the pour flows around every trace and pad with 0.3 mm moats.

What I should see: solid copper nearly everywhere, small islands silently deleted, thermal-relief spokes on THT pads. If some region stays empty, it's enclosed by traces with no way in — that's what the stitching vias fix next.

## Phase 2 — Stitching pass (~30–45 min, the meditative part)

Stitching vias tie the outer pours down into the inner planes so every patch of top/bottom copper is real low-inductance ground, not decoration.

**How to place a free via:** Place → **Via**, then click to drop; it takes the GND net from the pour beneath it. Even faster: click an existing GND via, **Ctrl+C**, then **Ctrl+V** repeatedly — pasted vias keep the net. Use my standard 0.6/0.3.

**Where to put them, in priority order for this board:**

1. **Anywhere the fill left a pad on an island or peninsula.** After filling, press B again and look for ratsnest lines still pointing at GND pads — each one gets a via next to it. My SW1/SW2 ground pads and the R13 ground are the known customers.
2. **Along the top edge near the antenna** — a row roughly every 3 mm at y ≈ 51–52, from x ≈ 54 to 78, dodging the module pads and the C9/C10/C14 links. This is straight from Espressif: "sufficient ground copper and dense ground vias near the antenna," on the board side, never past the edge.
3. **A loose ~5 mm grid across any large open pour areas** (my LAW 21) — bottom-left region, center gaps, the sensor corner. Precision is irrelevant; coverage is the point.
4. **Around the LDO's copper** — make sure U2's ground region carries 4–6 vias total into the planes (its heatsink path).
5. **The board perimeter** where pour exists, every ~5 mm — it keeps the outer pours and inner planes acting as one object at the edges.

Rules while stitching: don't line vias into tight rows closer than ~1 mm (hole-to-hole rule, and rows perforate the planes), nothing past the top edge, and refill (**B**) when done — every via punches a clearance hole that the fill must renegotiate.

## Phase 3 — Refill + full DRC loop (~30 min the first time)

1. **B** to refill (always refill before DRC — stale pours make DRC lie).
2. **Inspect → Design Rules Checker.** Tick **"Refill all zones before performing DRC"** and **"Test for parity between PCB and schematic"**. Run.
3. Read the three tabs. My targets: **Errors: 0. Unconnected items: 0.** Warnings: each one either gets fixed or consciously excluded (right-click → Exclude, and keep my habit of knowing why).
4. Expected findings on the first run: silkscreen collisions (fixed in Phase 4), possibly a few zone-clearance nits where a via sits tight. Click any violation to zoom to it; fix; refill; re-run. Loop until clean.
5. Open the schematic once more and run **ERC** — it should still be clean; parity between the two is what step 2's checkbox verified.

## Phase 4 — Silkscreen pass (~45 min)

Set up the view first: in the Appearance panel, dim or hide the copper layers and show **F.Silkscreen** — silk work goes fast when I can see it.

1. **Fix colliding reference designators.** The DRC silk warnings are my worklist (~35 items). Click a refdes, **M** to move, **R** to rotate; park each one beside its part where it's readable. Rotate, don't delete — my own rule. Keep text ≥1.0 mm / 0.15 mm thickness (the JLC legibility floor already in my constraints).
2. **Label the test points by function**, not number: Place → **Text**, layer F.Silkscreen, height 1.0–1.2 mm. Next to each pad: `5V` (TP1), `VSYS` (TP2), `3V3` (TP3), `VBAT` (TP4), `BSNS` (TP5), `SOIL` (TP6), `GND` (TP7, TP8), `EN` (TP9), `BOOT` (TP10), `TX` (TP11), `RX` (TP12). Future-me with a meter will thank present-me.
3. **The safety silk** (my LAW 27): big unmissable **+** and **−** beside J3's pins — the board's one true hazard is a reversed battery. Pin-1 marks at J2 and J3. `BOOT` beside SW1, `RESET` beside SW2 (schematic refs — SW1 is BOOT). `PWR` at D2, `CHG` at D3. Check D1/D4 cathode marks survived from the footprints.
4. **Nothing over pads, nothing on the BME280** — no silk on or near U4's vent, and DRC's silk-over-pad check catches the rest.
5. Optional flourishes: board name + rev + date somewhere open (`PlantMonitor RevA 2026-08`), and my logo lives at `logos:muffinByteLogo` — Place → Footprint if I want it. If I'd rather JLC's order number not land somewhere ugly, place a text reading exactly `JLCJLCJLCJLC` where I *want* it — their system replaces it and prints nothing elsewhere.
6. Re-run DRC: silk warnings should now be at or near zero.

## Phase 5 — 3D sanity + pre-order gauntlet

1. **Alt+3** for the 3D viewer. Slow lap around the board: USB nose overhang looks right, JST faces flush-ish, antenna hanging clean off the edge, nothing tall crowding the USB plug path, silk readable.
2. **Generate fabrication files with the JLC plugin** (I installed kicad-jlcpcb-tools): toolbar/Tools → External Plugins → JLCPCB Tools → Generate. It produces the gerber zip, BOM, and CPL (placement) files together, with LCSC numbers from my schematic fields. (Manual fallback: File → Fabrication Outputs → Gerbers — include F.Cu, In1.Cu, In2.Cu, B.Cu, both masks, both silks, Edge.Cuts — plus Drill Files. The plugin is easier and JLC-shaped.)
3. **In the plugin/BOM preview, verify the exclusions:** C3, C4 (DNP), all 12 TPs, H1–H4 — none of them on the BOM or placement file.
4. **Upload to JLC.** Board settings: **4 layers · JLC04161H-7628 · 1.6 mm · impedance control: No** · surface finish HASL (lead-free) is fine · everything else defaults.
5. **Assembly settings:** the module (C2913198) is **Standard PCBA tier**, so pick Standard assembly. Then the check my rulebook calls the classic victim-catcher: **in JLC's component placement preview, verify rotation and polarity of every polarized part** — D1, D2, D3, D4, U1–U6, J1, Q1–Q3, SW1/SW2. JLC's rotations sometimes differ from KiCad's by 90°/180°; their preview render is the truth. LEDs especially.
6. **Live stock check** at upload (my BOM notes the 10 k alternate C98220 if C25804 is out again).
7. **Freeze and commit:** copy the ordered gerber zip, BOM, and CPL into `fabrication/revA/`, then git commit and push — my rule 32. The order confirmation email PDF is worth saving there too.

Then the board is out of my hands for a week, `BringUp_Guide.md` becomes the active document, and step 1 of that guide — meter the battery plug before it ever touches J3 — is already written in my own hand.

---

*Click paths verified against KiCad 10 · companion to `Routing_Guide_RevA_4Layer.md` §8's definition of done.*
