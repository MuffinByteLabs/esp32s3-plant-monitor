# Reading What the Factory Sends Back — the Production-File Confirmation

*A primer for the confirmation step, written for someone approving their first board. Companion to `Fabrication_File_Primer.md` (which covers what **I** send) — this one covers what **JLC** sends back and asks me to approve. Worked examples are from the Rev A package, production no. `12792098A_Y3`, 2026-08.*

---

## 0. The single most important idea

**The confirmation package is not my upload — it's what their CAM engineer did to my upload.** A human at JLC opened my gerbers in their CAM tool, cleaned them, compensated them for their process, drew a panel around my board, and exported new files. Those new files — not mine — are what the machines run.

So the question at this step is never "is my design right?" (too late — review that before ordering). It's two narrower questions:

1. Did anything on **my board** change beyond routine process edits?
2. Is the **panel** they drew around it compatible with my board — rails, scores, and marks in the right places?

Rev A passed question 1 completely and failed question 2 (top rail under the antenna overhang). That's typical: the panel is where confirmation problems live, because it's the one thing they created that I never drew.

---

## 1. What arrives, and what each thing is

| Item | Example from Rev A | What it is | Attention |
|---|---|---|---|
| `PCB_<prod-no>.zip` | `PCB_12792098A_Y3.zip` | **My own gerbers echoed back**, repackaged | Diff against my frozen zip — must be identical |
| Loose files, no extensions, 2–4 letter names | `tl`, `bo`, `drl`, `vcut`… | **The CAM working files.** The actual production artwork | This is what I'm approving. Read these |
| A small `.json` | `4te.json` | The order as their system sees it: layers, thickness, finish, via treatment, panel size — plus my remark text, proving it was received | Worth one look. **GBK-encoded Chinese** — open with GBK/GB18030, not UTF-8, or the strings are mojibake |
| `<prod-no>.tgz`, `.ddw` | `12792098a_y3.tgz` | Their internal CAM database (Genesis/InCAM job) | Ignore |

The production number (`12792098A_Y3`) is their name for my order — it appears in filenames and on the QR label, and is what to quote in any email.

---

## 2. The decoder — CAM file names → my layers

The working files use jlccam's short names. Mapping, top of board to bottom:

| File | Meaning | My layer | Notes |
|---|---|---|---|
| `tl` | top layer | `F.Cu` | copper |
| `l2` | layer 2 | `In1.Cu` (GND1) | verified l2=In1, l3=In2 on Rev A by overlay — order is top-down |
| `l3` | layer 3 | `In2.Cu` (GND2) | |
| `bl` | bottom layer | `B.Cu` | |
| `to` / `bo` | top/bottom **overlay** | `F.Silkscreen` / `B.Silkscreen` | "overlay" = silk in CAM-speak |
| `ts` / `bs` | top/bottom **solder** | `F.Mask` / `B.Mask` | mask *openings* — negative, same as always |
| `gtp` / `gbp` | top/bottom paste | `F.Paste` | stencil source; may only exist inside the `.tgz` |
| `sk` | 塞孔 — via **plug map** | (no equivalent) | one flash per via that gets plugged. Count it (see §5) |
| `ko` | keep-out / profile | `Edge.Cuts` + panel | board outline **and** panel outline **and** the routed pocket, all in one layer. The panel drawing lives here |
| `drl` | all drills | `PTH.drl` + `NPTH.drl` | merged into **one gerber** (not Excellon): round holes as flashes, slots as stroked lines, plus rail tooling holes. Sizes are compensated (§4) |
| `vcut` | V-score lines | (no equivalent) | score lines + dimension text. Lines extend past the panel and text sits outside it — normal, not a bug |
| `qrt` / `qrb` | QR + production code, top/bottom | (no equivalent) | traceability marks. Must be **on the rails**, never on my board — especially with Remove Mark ordered |

Bottom-side files (`bo`, `bs`, `bl`) display mirrored in a viewer — correct, since they're viewed through the board.

---

## 3. If I read only four things

1. **`ko` + `vcut` — the panel drawing.** Where is my board in the panel, how wide are the rails, where are the routed gaps, where do the scores run? Hold every edge-overhanging component (U3 antenna, J1 shell, J2/J3) against it: **nothing coplanar may sit under an overhang during assembly.** This is where Rev A's problem was.
2. **`drl` — the holes.** Count matches, positions match, and the size changes follow the compensation table in §4.
3. **`to` + `bo` — the silk.** The only edits should be clipping (over pads, at the outline) and my order-number token honored per the order (Rev A: `JLCJLCJLCJLC` stripped, ID block untouched). Anything *added* to board silk is a red flag.
4. **The echoed zip** — byte-diff against `fabrication/revA/GERBER-*.zip`. Identical or stop.

Viewers: KiCad's Gerber Viewer opens the extensionless files fine (File → Open, filter "All files"), as does gerbv. The CAM files are in inches with 2.6 format while mine are metric — viewers reconcile units automatically; don't panic at the raw coordinates.

---

## 4. Drill compensation — expected, not an error

The `drl` file shows **tool sizes, after compensation**, not my finished sizes. Plated holes are drilled oversize because the plating grows the barrel inward; the finished hole lands back on my number. Rev A's observed mapping:

| Mine (finished) | Theirs (drill) | Delta | Why |
|---|---|---|---|
| 0.2 / 0.3 / 0.4 via | 0.2 / 0.3 / 0.4 | 0 | vias drilled at nominal — finished via ID doesn't matter functionally |
| 0.6 slot (J1 shield) | 0.75 slot | +0.15 | plating compensation |
| 0.75 (J2/J3 pins) | 0.90 | +0.15 | plating compensation |
| 1.0 (TP8/11/12) | 1.15 | +0.15 | plating compensation |
| 0.65 NPTH | 0.70 | +0.05 | routing/tool preference |
| 3.2 NPTH (H1–H4) | 3.25 | +0.05 | routing/tool preference |
| — | 4 × Ø2.05 | new | tooling holes, on the rails — expected |

First-timer panic #1 is "they changed all my hole sizes!" They didn't. They changed the *drill* so the *hole* comes out right.

---

## 5. Normal CAM edits — the whitelist

Everything on this list appeared on Rev A and is routine. Seeing these is evidence of a competent CAM pass, not a problem:

- **Outer copper grown a hair** (~2% area, everywhere, uniformly): etch compensation.
- **Inner-plane pullback re-clipped** at the board edge, and **anti-pads enlarged** around a few through-holes: registration margin.
- **Mask openings grown** ~0.05–0.08 mm per side; **openings added** over NPTH holes (H1–H4, connector anchors) and a mask-free ring traced along the board edge and score lines: keeps mask from flaking at cut edges and hole rims.
- **Silk clipped** wherever it crossed exposed copper or the outline; line widths normalized.
- **Order-number token removed** (when "Remove Mark — token" was the order); QR + production code printed on the rails, both sides.
- **Fiducials and Ø~2 mm tooling holes on the rails.**
- **`sk` plug-map count = via count.** Rev A: 157 flashes = 111 × 0.3 + 46 × 0.4 ✓. The 12 × 0.2 thermal vias in U3's pad are deliberately *not* plugged — via-in-pad stays open.

## 6. Red flags — reply instead of approving

- The echoed zip differs from my frozen upload.
- Copper geometry moved, appeared, or vanished anywhere (beyond the uniform hairline growth above). Same for any drill.
- Anything **added to board silk** — date codes, serials, a QR on the board proper — when Remove Mark was ordered.
- **Rails, tabs, or v-cut webs under an overhanging component** (Rev A: 11 mm top rail 2 mm from the edge, under a 6.5 mm antenna overhang → requested a routed cutout). The remark text being present in the `.json` does not mean the panel implements it. Check the geometry.
- V-score running through or hair-close to plated features (JLC minimum ~0.4 mm copper-to-score).
- Plug count ≠ via count when plugged vias were ordered.
- Layer order suspicion: overlay `l2`/`l3` against `In1`/`In2` on an asymmetric feature. Swapped inners build a mirrored stackup.
- `.json` disagreeing with the order: layer count, thickness, finish (无铅喷锡 = lead-free HASL), via treatment (过孔塞油 = plugged), colors.

---

## 7. The ritual (ten minutes)

1. Download everything; unzip nothing over my originals.
2. Byte-diff the echoed zip against `fabrication/revA/`. Identical → continue.
3. Open `ko` + `vcut` + `tl` together. Measure rail widths and gaps. Walk each board edge asking *what overhangs here, and what's under it during reflow?*
4. Open `drl`, count holes, check sizes against §4's pattern.
5. Overlay `to`/`bo` on my silks. Confirm the mark decision was honored, and nothing was added.
6. Skim `ts`/`bs` (expect §5 edits only), count `sk`.
7. Decode the `.json` (GBK) and read the spec fields and my remark back.
8. Approve — or reply listing exact changes wanted, with panel coordinates, and re-run this ritual on the revised files.
9. Freeze the confirmation package next to the order files (`fabrication/revA/production_confirmation/`), git commit. Future-me debugging a bad board needs to know exactly what was approved.

---

## 8. Rev A worked record — 2026-08-25

Panel 72.5 × 70.5 mm; board at x 5–67.5, y 13–57.5. Rails: 11 mm top/bottom behind 2 mm routed gaps; 5 mm left/right attached by v-score at the board edge, no gap. Scores at x = 0, 5.0, 67.5, 72.5 (vertical only); corners routed, R4.25 preserved. QR + code on top rail, both sides. All eight board layers verified against the order by pixel overlay: only §5 whitelist edits. Drills 183 + 4 slots all matched (§4 table). Token stripped, ID block intact, nothing added to board silk.

**Verdict was: request modification, not approve** — routed cutout in the top rail across x ≈ 25–48 (antenna overhang zone), QR relocated out of the cutout. Noted, accepted as-is: USB-C shell overhang rests on the flush v-cut left rail (low risk); full-length score 3.4 mm from C11 in place of the requested tab keep-away (machine separation, gentler than snapping).
