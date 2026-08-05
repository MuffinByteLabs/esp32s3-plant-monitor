# JLCPCB Manufacturing Capabilities — Reference Copy

*Captured from jlcpcb.com by me, 2026-08-02. Like stock and Basic/Extended status, capabilities are **live values** — re-skim the site before each order. Sections: [Rev A quick sheet](#what-this-means-for-rev-a) · [Rigid PCB](#rigid-pcb-capabilities) · [Assembly](#pcb-assembly-capabilities) · [Stencil](#smt-stencil-capabilities) · [Flex](#flexible-pcb-capabilities).*

---

## What this means for Rev A

My board (2-layer, 1.6 mm, green, JLC assembly) against JLC's limits — everything fits, with margin:

| Mine (Rev A) | JLC limit | Margin |
|---|---|---|
| Track/clearance 0.2 / 0.2 mm | 0.10 / 0.10 (1 oz) | 2× |
| Routing via 0.6 / 0.3 | 0.25 / 0.15 min | comfortable — and **no surcharge** (extra cost only for 0.15 holes, or 0.2/0.25 holes with pad < 0.45) |
| ESP32 module thermal vias 0.6 / 0.2 | see above | **no surcharge** — hole 0.2 but pad 0.6 ≥ 0.45 ✓ |
| Copper-to-edge 0.5 | ≥ 0.2 (routed) | 2.5× |
| Pad hole-to-hole (KiCad set 0.5) | 0.45 pad holes / 0.2 via holes | ✓ |
| PTH hole to track — KiCad "copper to hole" **set 0.3** | 0.28 min / 0.35 recommended | ⚠ was 0.25 → bumped to 0.3 (2026-08-02) |
| NPTH holes: USB-C pegs Ø0.65, mounting Ø3.2 | ≥ 0.5 | ✓ |
| Silkscreen 1.0 mm text / 0.15 line | 1.0 / 0.15 minimum | at the floor — don't go smaller |
| Silk near pads | clipped within 0.15 of a pad | expect clipping, not an error |
| 0603 passives, 0.65 mm LGA, 1.27 mm module | Economic: 0402 / 0.4 mm pitch min | ✓ all parts qualify dimensionally |
| All parts top side | Economic = single-sided placement | ✓ |
| 2 L / 1.6 mm / green | Economic PCBA: HASL or ENIG, qty 2–50 | ✓ sweet spot |

Notes for ordering day: **reflow is 255 ± 5 °C and not adjustable** on Economic (all my parts are fine with that; the battery is never assembled). The ESP32-S3 module (C2913198) is a **Standard-tier part** per PROJECT_STATUS — expect the Standard assembly flow/pricing for that line. Board size 10×10 mm – 470×500 mm covers me trivially. No stencil purchase needed when JLC does the assembly — the stencil section below matters only if I ever hand-paste at home.

---

## Rigid PCB Capabilities

### General
| Item | Capability |
|---|---|
| Layer count | 1–32 |
| Controlled impedance | 4+ layers only, ±10% (not available on 2-layer) |
| Material | FR-4 grade A (Nan Ya, KB, Shengyi…); also 1-layer aluminum-core, 1-layer copper-core, 2-layer RF (Rogers/PTFE) |
| FR-4 dielectric constant | 4.5 (2-layer) · 7628 prepreg 4.4 · 3313 prepreg 4.1 · 2116 prepreg 4.16 |
| Max dimensions | 2-layer FR4: 670×600 mm (up to 1020×600 special); ≥0.8 mm thickness assumed, thinner max 599×497 |
| Min dimensions | FR4: 3×3 mm (castellated/plated edge: 10×10; ≥0.6 mm thickness, thinner = manual review) |
| Dimension tolerance | ±0.1 mm precision / ±0.2 regular (CNC routed); ±0.4 V-scored |
| Thickness | FR4: 0.4/0.6/0.8/1.0/1.2/**1.6**/2.0 mm (2.5+ = 12+ layers only) |
| Thickness tolerance | ≥1.0 mm: ±10% (1.6 → 1.44–1.76) · <1.0 mm: ±0.1 |
| Outer copper | 2-layer: 1/2/2.5/3.5/4.5 oz · multilayer: 1/2 oz |
| Inner copper | 0.5 (default) / 1 / 2 oz |
| Soldermask | LPI; green, purple, red, yellow, blue, white, black |
| Surface finish | HASL (leaded/lead-free), ENIG, OSP (copper-core only); 6+ layer & RF = ENIG only |

### Drilling & holes
| Item | Capability |
|---|---|
| Drill diameter | 2-layer: 0.15–6.3 mm (≥6.3 CNC-routed; **0.15 costs more**) |
| Hole size tolerance | PTH +0.13/−0.08 mm; press-fit ±0.05 (multilayer ENIG only); PTH ≥0.5 recommended to avoid mask/tin plugging |
| Hole plating | 18 µm average; hole position ±0.05 mm |
| Blind/buried vias | **Not supported** (through-hole only) |
| Min via hole/diameter | 2-layer: 0.15 / 0.25. Diameter ≥ hole + 0.1 (0.15 preferred). **Preferred min hole 0.2.** Surcharge: 0.15 hole any diameter; 0.2/0.25 hole with diameter < 0.45 |
| Min NPTH | 0.5 mm (draw in mechanical/keepout layer) |
| Min plated slot | 2-layer: 0.5 mm wide (drawn as pad; length ≥ 2× width) |
| Min non-plated slot | 1.0 mm (draw outline in mechanical layer); tolerance: plated +0.13/−0.08, non-plated ±0.2 |
| Via hole-to-hole spacing | 0.2 mm |
| **Pad hole-to-hole spacing** | **0.45 mm** |
| Castellated holes | Ø≥0.5, edge ≥1, hole-hole ≥0.5, board ≥10×10 & ≥0.6 mm thick |
| Plated edges | 10×10 min, ≥0.6 thick, ENIG only, ≥3 breaks for support tabs |
| Blind slot | W≥1.0, D≥0.2, annular ≥0.3, safety ≥0.2, remaining ≥0.2; 2–32 L, ≥0.8 mm |
| Backdrill | 4–32 L only |
| Rectangular (sharp-corner) holes/slots | **Not supported** — round the corners |

### Traces & clearances
| Item | Capability |
|---|---|
| Min track/space (1 oz) | 1–2 layer: **0.10/0.10 mm (4/4 mil)**; multilayer 0.09/0.09 |
| Min track/space (2 oz) | 2-layer: 0.16/0.16 |
| Track width tolerance | ±20% |
| PTH annular ring (2-layer, 1 oz) | recommended ≥0.25, absolute min 0.18 |
| NPTH pad annular ring | ≥0.45 recommended (thinner may vanish — sealing film needs a 0.2 copper pullback) |
| BGA | pad ≥0.2 (0.2–0.25 needs ENIG); pad-to-trace ≥0.1 |
| Hatched grid | ≥0.25/0.25 |
| Same-net track spacing | ≥0.25 |
| Pad-to-track clearance | ≥0.1 |
| SMD pad-to-pad (different nets) | ≥0.15; min SMD pad 0.25×0.25 |
| Via hole to track | ≥0.2 |
| **PTH hole to track** | **0.35 recommended, 0.28 minimum** ← drove my KiCad copper-to-hole = 0.3 |
| NPTH to track | ≥0.2 |

### Soldermask & legend
| Item | Capability |
|---|---|
| Mask expansion | 1:1 pad:opening supported (LDI, since 2025-06); keep ≥0.09 opening-to-trace |
| Soldermask bridge | 1 oz: ≥0.10 pad spacing (green/red/yellow/blue/purple), ≥0.13 (black/white) |
| Plugged vias | mask-filled; no openings either side; ≥0.35 from other openings; ≤0.5 diameter |
| Via-in-pad process | epoxy or copper-paste filled & capped, Ø0.15–0.55 (default on 6+ layers) |
| Silk line width | ≥0.15 mm |
| Silk text height | ≥1.0 mm (40 mil); width:height 1:6 preferred |
| Pad to silkscreen | ≥0.15 (closer = clipped) |

### Outline
| Item | Capability |
|---|---|
| Routed | copper to edge ≥0.2; tolerance ±0.2 (±0.1 high precision, needs ≥50×50 + 3 tooling holes ≥Ø1.5) |
| V-cut | copper to edge ≥0.4; tolerance ±0.4; ≥0.6 thick; panel 70×70–475×475; 25° groove |
| Mouse-bite panels | edge clearance ≥0.2; spacing 1.6/2 mm; JLC SMT: 5 mm rails, 2 mm tooling holes, 1 mm fiducials at 3.85 from edge; bite Ø0.5–0.8, tab ≥4–5 mm |
| Panel spacing | ≥2 mm |
| Circular boards in panel | single board ≥20×20 |

---

## PCB Assembly Capabilities

| Item | Economic | Standard |
|---|---|---|
| Placement | Single-sided (SMT/THT) | Single & double-sided |
| Layers | 2, 4, 6 | 1–32 |
| Thickness | 0.8–1.6 mm | no limit |
| Single PCB size | 10×10 – 470×500 mm | 70×70 – 460×500 mm |
| Panel size | 10×10 – 250×250 | 70×70 – 250×250 |
| Order volume | 2–50 pcs | 2–80 000 pcs |
| Finish / color | per Economic table below | no limit |
| Delivery format | single, panel w/ mouse bites | + V-cut panels |
| Stackup | standard only | all |
| Gold fingers / castellated / edge plating | not supported | supported |
| Edge rails / fiducials | not necessary | necessary |
| Min package | **0402** | 0201 |
| Min IC pin spacing | **0.4 mm** | 0.35 mm |
| Min BGA pitch | 0.5 mm | 0.35 mm |
| Reflow | **255 ± 5 °C, not adjustable** | 240 ± 5 °C |
| SPI (paste inspection) | no | yes |
| AOI / visual | yes | yes |
| X-ray | BGA-type parts | same |
| Build time | 1–3 days | ≥4 days |

**Economic PCBA — allowed PCB specs** (layers · thickness · color · finish · qty):

| Layers | Thickness | Colors | Finish | Qty |
|---|---|---|---|---|
| 2 | 0.8 | green | HASL (LF or leaded) | 2–30 |
| 2 | 1.0 / 1.2 | green, black | HASL | 2–30 |
| **2** | **1.6** | **green** | **HASL or ENIG** | **2–50** ← Rev A |
| 2 | 1.6 | black | HASL | 2–50 |
| 2 | 1.6 | blue, purple | HASL | 5–30 |
| 2 | 1.6 | red, white | leaded HASL only | 5–30 |
| 4 | 1.0 | green | HASL | 2–30 |
| 4 | 1.2 | green | HASL | 2–50 |
| 4 | 1.6 | green | HASL or ENIG | 2–50 |
| 6 | 1.6 | green | ENIG | 2–30 |

---

## SMT Stencil Capabilities

*(Only needed if I ever hand-paste boards myself — JLC assembly doesn't require ordering one.)*

| Item | Capability |
|---|---|
| Types | framework & non-framework |
| Material | 304 HTA stainless steel, precision laser cut, tolerance ±0.003 mm |
| Min aperture | >0.08 mm |
| File formats | Gerber (paste layers), DXF |
| Thickness | JLC-selected, or by customer. Standard (free): 0.10/0.12/0.15/0.18/0.20. Special (paid): 0.03–0.08, 0.25–0.5 |
| Non-framework size | standard 280×380–700×600; custom up to 650×580 (1:1-as-PCB = custom size) |
| Framework size | min 400×300 (valid 240×140); square max 736×736 (valid 500×500); rect max 1500×500 (valid 1300×320) |
| Options | nano-coating (all); step stencil & ultrasonic-resistant adhesive (framework only) |
| Sides | top / bottom / both-on-one / both-separate |
| Process | solder paste or red glue |
| Polishing | sanding, etching, electropolishing (best for ≤0.5 mm pitch & BGA) |
| Fiducials | none / etched through / etched half |
| Build time | fastest 12 h; ≤280×280 ships with PCBs (larger ships separately on express) |

---

## Flexible PCB Capabilities

*(Not used in this project — kept for reference.)*

| Item | Capability |
|---|---|
| Layers | 1, 2, 4 (no rigid-flex yet) |
| Stackups | 1L/2L on 25 µm PI; 1L/2L on 50 µm PI (tear-resistant, impedance-suitable); 1L/2L transparent PET 36 µm; 4L with 1/3, 0.5, 1 oz copper |
| Max size | 234×490 regular (250×600 absolute, with rails + support confirmation) |
| Finished thickness | 25 µm-core: 1L 0.07/0.11, 2L 0.11/0.12/0.2 · 50 µm-core: 1L 0.12, 2L 0.19 · transparent: 0.14/0.24 · 4L: 0.2–0.45 |
| Copper | single-sided 0.5/1 oz; double & 4L: 0.33/0.5/1 oz |
| Process / finish | LDI dry film; ENIG 1u"/2u" only |
| Thickness tolerance | ±0.05 (stiffener ≤0.3), ±0.1 (0.3–1.0), ±10% (>1.0); gold fingers ±0.03 |
| Holes | Ø0.1–6.5 (PTH ≤5 recommended); tolerance ±0.08; plated slot ≥0.5; NPTH slot unlimited (0.2 copper clearance) |
| Castellated | Ø≥0.3, edge ≥0.5, hole-hole ≥0.4 |
| Vias | regular 0.3/0.55; extreme 2L 0.10/0.3, 4L 0.15/0.35 (paid); diameter ≥ hole+0.2 (0.25 better) |
| PTH annular | ≥0.25 rec, 0.18 absolute |
| Track/space | 0.33 oz: 3/3 mil (2/2 absolute) · 0.5 oz: 3.5/3.5 · 1 oz: 4/4; tolerance ±20% |
| Clearances | via ring-track ≥0.1; exposed pad-track ≥0.15; NPTH-copper ≥0.2; BGA pad ≥0.25, pad-track ≥0.2 |
| Coverlay | yellow (recommended)/black/white/transparent; expansion 0.1/side; opening-trace ≥0.15; keep over vias; white +10–18 µm/side |
| Solder bridge | ≥0.5 or it's removed |
| Silk | height ≥1, line ≥0.15, pad clearance ≥0.15 |
| Outline (laser) | copper-edge ≥0.3, copper-slot ≥0.3, tolerance ±0.1 (±0.05 on request); gold finger to edge 0.2 |
| Panels | spacing 2 (3 w/ metal stiffener); 5 mm rails w/ copper; fiducials 1 mm / tooling 2 mm, center 3.85 from edge; tabs 0.7–1.0; max 234×490 |
| Stiffeners | PI 0.1–0.25; FR4 0.1–1.6 (avoid — chips); steel 0.1–0.3 (flat, but magnetic); 3M9077 / 3M468 / tesa8854 tape; EM shield film 18 µm |
| Impedance | εr: PI 3.3, coverlay 2.9; **no measurement/control** — width-controlled only |
| Other constraints | same as rigid for holes, traces, mask, silk |
