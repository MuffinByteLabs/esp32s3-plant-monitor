# What a Factory Actually Needs to Build My Board

*A primer for Rev A. Written for someone ordering their first board. Companion to `docs/Finishing_Guide_RevA.md` Phase 5, which assumes I already know all of this.*

---

## 0. The single most important idea

**My design files never leave my computer.** JLC never sees `ESP32S3_PlantMonitor.kicad_pcb`. What I send is a set of *derived* files — flattened, dumb, machine-readable descriptions of shapes and coordinates. They contain no nets, no schematic, no design intent, no rules.

This matters because it explains every quirk below. When I wonder "how does the factory know X?", the answer is almost always either *it's in one of these files*, or *I told them in a dropdown at order time*, or — the dangerous one — *it doesn't, and it will guess*.

---

## 1. Two different services, two different file sets

| | Bare board fabrication | Assembly (PCBA) |
|---|---|---|
| What I get | A blank PCB — copper, mask, silkscreen, holes | That board with 51 components soldered on |
| Files needed | Gerbers + drill files (one zip) | BOM + CPL/position file |
| Who does it | The board shop | The assembly line, often a different building |

I can order the first without the second. I cannot order the second without the first. My JLC plugin generates all four things in one click, which hides the distinction — worth knowing it's there.

---

## 2. Gerbers — the board's artwork

### What a Gerber actually is

A Gerber file is a **2D vector image**, one per layer. It's a text file that says "select this aperture (a shape — round, rectangular, custom), move to this coordinate, flash it or drag it." Stack all the layers up in register and the result is a complete picture of the board.

The format is RS-274X, now extended as **Gerber X2**, which adds metadata attributes — the file can now declare "I am the top copper layer" and "this pad is an SMD pad, this one is a via." KiCad exports X2 by default. This is why modern fabs auto-identify my layers instead of demanding specific filenames, and why the old ritual of carefully naming files `.GTL`, `.GBL`, `.GTS` has largely died.

**A Gerber contains shapes and nothing else.** No thickness. No material. No copper weight. No net names. No part numbers. Two boards with identical Gerbers can be 0.8 mm or 2.0 mm thick, 2-layer or 4-layer, ENIG or HASL. All of that comes from the order form.

### The layers my board needs

| Gerber | What it is | Failure mode if wrong |
|---|---|---|
| `F.Cu` | Top copper | — |
| `In1.Cu` | GND1 plane | Omitted → they build a 2-layer board |
| `In2.Cu` | GND2 plane | Layers swapped → stackup inverted |
| `B.Cu` | Bottom copper | — |
| `F.Mask` / `B.Mask` | Solder mask openings | See below |
| `F.Silkscreen` / `B.Silkscreen` | White printed ink | Text over a pad gets clipped by the fab |
| `Edge.Cuts` | Board outline | **The single most common upload rejection** |

### Solder mask is backwards from what I'd expect

The green coating covers the *whole board*. The mask Gerber marks where it is **removed**. So a shape in `F.Mask` is a *hole* in the green, exposing copper so I can solder to it.

The fab also applies **mask expansion** — typically ~0.05 mm larger than the pad all round — so that slight registration error between the copper and mask layers doesn't leave green creeping onto a pad. I don't draw this; KiCad and the fab handle it.

Vias are usually **tented** — mask left over them so they're sealed. That was the plan for my 154 vias, but JLC no longer offers Tented on 4-layer, so Rev A was ordered **Plugged**; untented vias under a component can wick solder.

### Edge.Cuts must be one closed loop

This is where first orders die. The outline has to be a single, continuous, closed shape with no gaps and no double-drawn segments. A 0.01 mm gap between two line endpoints means the fab's software cannot determine what is board and what is not, and my order goes on hold with an email asking me to fix it.

My board is 62.5 × 44.5 mm with R4.25 rounded corners, drawn as four arcs and four lines. If the arcs and lines don't meet exactly, that's the failure. KiCad's DRC has an "unconnected board outline" check — it's worth running deliberately before export.

### Layers I have but won't send

`F.Fab` / `B.Fab` are documentation layers for my own use — assembly drawings, part outlines. `User.Comments`, `User.Drawings` likewise. These never go to the fab. `F.Paste` / `B.Paste` describe stencil openings; I'd need them if I were hand-assembling with a paste stencil, but JLC generates their own for machine assembly.

---

## 3. Drill files — the holes

Separate from Gerbers, in **Excellon** format. Structure is a tool table followed by coordinates:

```
T1C0.200    ; tool 1 = 0.2 mm drill
T2C0.300    ; tool 2 = 0.3 mm drill
...
T1
X64390Y58710   ; drill a 0.2 mm hole here
X64390Y59980
```

### Plated vs non-plated — and why they're separate files

- **PTH (plated through-hole)** — the hole barrel is copper-plated, so it conducts between layers. Every via on my board, plus the J1/J2/J3 connector pins and the THT test points.
- **NPTH (non-plated)** — a bare hole, no copper. My H1–H4 M3 mounting holes.

They're separate files because they happen at different points in the process. PTH holes are drilled before the plating bath; NPTH holes must be excluded from plating, usually by drilling them afterwards. If my mounting holes end up in the PTH file, I get copper barrels in them — which on a grounded board means my M3 screws are now electrically connected to the plane. Sometimes that's wanted; on my board it isn't.

### My board's drill inventory

| Diameter | Count | What |
|---|---|---|
| 0.2 mm | 12 | U3 module thermal vias (PTH) |
| 0.3 mm | 96 | standard vias (PTH) |
| 0.4 mm | 44 | power/doubled vias (PTH) |
| 0.75 mm | 5 | J2/J3 JST pins (PTH) |
| 1.0 mm | 3 | THT test points TP8/TP11/TP12 (PTH) |
| ~3.2 mm | 4 | H1–H4 mounting holes (**NPTH**) |

A **drill map** or drill report is a human-readable PDF/Gerber showing hole locations with symbols and a size table. Optional at JLC; standard practice at traditional fabs.

---

## 4. BOM — what to buy

A CSV. For JLC the columns that matter are:

| Column | Purpose | Example from my board |
|---|---|---|
| Designator | Which parts | `R5,R6,R7,R13,R16` |
| Quantity | How many | `5` |
| Comment / Value | Human sanity check | `10k` |
| Footprint | Human sanity check | `R_0603_1608Metric` |
| **LCSC Part #** | **The actual order** | `C98220` |

Identical parts are grouped onto one line with a comma-separated designator list.

**The LCSC number is the only column that truly binds.** Value and footprint are there so a human reviewer can spot when I ask for C98220 but write 100k — the machine buys whatever the LCSC number says. This is why a typo'd part number produces a board that's fully assembled and completely wrong.

All 51 of my fitted parts already carry LCSC numbers. Parts that must **not** appear here: C3 and C4 (marked DNP), all 12 test points, and H1–H4.

**Stock is checked at order time, not design time.** A part that existed when I drew the schematic may be out of stock today. The 10 k proved it: C25804 was the original choice, it failed JLC's pre-order sourcing check on 2026-08-18, and the BOM moved to C98220 before the order went through. Carrying a named alternate in the BOM notes is what made that a same-day fix instead of a lost week.

---

## 5. CPL / position file — where to put it

Also a CSV. One line per placed component:

| Column | Meaning |
|---|---|
| Designator | `R5` |
| Mid X / Mid Y | Coordinates of the footprint's origin |
| Layer | `top` or `bottom` |
| Rotation | Degrees |

### Rotation is where boards get built backwards

This is the single biggest first-order hazard, so it's worth understanding *why* it happens rather than just being told to check.

KiCad stores a rotation relative to how *KiCad's* footprint library defines pin 1. The assembly machine feeds parts from a reel, and the part's orientation in that reel is defined by *the supplier's* datasheet convention. For many part types those two conventions differ — commonly by 90° or 180°. JLC maintains an internal rotation-offset table for parts in their library, but coverage isn't perfect, and it can't know about my custom footprints.

The consequence is specific and expensive: a diode, LED, electrolytic capacitor, or IC placed exactly where it belongs but rotated 180°, so pin 1 is at the wrong corner. The board looks perfect and doesn't work.

**The only reliable check is their placement preview render** — an actual picture of my board with the parts drawn on. Compare it against my own 3D view, part by part, for everything polarized: D1–D4, U1–U6, J1, Q1–Q3, SW1/SW2. LEDs are the classic casualty.

### The origin question

Coordinates need a reference point. JLC expects them relative to the board's bottom-left corner or a defined origin. KiCad exports relative to its drill/place origin if I've set one, or the page origin if I haven't. My plugin handles the conversion — but if I ever export manually, this is the setting that silently shifts every part by the same offset.

---

## 6. Things that are settings, not files

None of this is in any file I upload. It's all dropdowns.

**Bare board:**

- Layer count — 4
- Dimensions — read automatically from Edge.Cuts
- Thickness — 1.6 mm
- Stackup — `JLC04161H-7628`. Note this means **1 oz (35 µm) outer copper and 0.5 oz (~15 µm) inner** — matching the 0.035 / 0.0152 numbers in my stackup
- Material — standard FR4
- Surface finish — lead-free HASL is fine for my board; ENIG costs more and is flatter (matters for fine-pitch parts)
- Solder mask colour, silkscreen colour
- Minimum track/spacing spec — affects price; my 0.2 mm minimum is well inside the standard tier
- Impedance control — **No**, per my decision (USB is Full Speed, 12 Mbps)
- Via treatment (plugged — Tented is not offered on 4-layer), gold fingers (no), castellated holes (no)
- Electrical test — flying-probe continuity check; standard on multilayer

**Assembly:**

- Which side — top only, all 53 parts are on F.Cu
- Assembly tier — **Standard**, forced by the module (C2913198)
- Quantity
- Tooling holes / edge rails — the machine needs somewhere to grip; JLC adds rails if needed

---

## 7. Optional files I'll hear about

| File | What it does | Do I need it? |
|---|---|---|
| **IPC-D-356 netlist** | Machine-readable "what should connect to what". The fab compares it against my Gerbers and flags mismatches before manufacturing | Optional, genuinely useful insurance, KiCad exports it |
| **ODB++ / IPC-2581** | Single-file formats bundling copper, drill, netlist and stackup together | Some fabs prefer them; Gerbers remain universal |
| **Fabrication drawing** | Dimensioned drawing with hole table and notes | Standard at traditional fabs, not needed for JLC's automated flow |
| **Stencil files** | Paste layers, for hand assembly | Not needed — JLC makes their own |
| **STEP / 3D model** | Not a fab file at all | For designing an enclosure |
| **Panel drawing** | If I want multiple boards in an array | Not for prototype quantities |

---

## 8. What happens after I press upload

1. **Automated DFM check** — software looks for unclosed outlines, clearances below spec, missing layers. Fast, and where most first-timer problems surface.
2. **Engineering review** — a human looks at anything ambiguous. This is where phantom CPL entries with no BOM line generate a query email, and where a 24-hour delay comes from.
3. **Parts sourcing** — they pull my LCSC numbers from stock.
4. **Fabrication** — the bare boards get made.
5. **Assembly** — and this is when my placement preview approval matters, because after this point a wrong rotation is a scrapped board.

---

## 9. My actual sequence

Everything above happens automatically from one plugin run. What I control is the state of the board *before* that run:

1. DRC clean, zones refilled.
2. Silkscreen pass done.
3. Exclusion flags set on C3, C4, TP1–TP12, H1–H4 — **after** my last F8.
4. Run the JLC plugin → gerber zip, BOM, CPL.
5. Upload. Set the board options in §6.
6. **Check the placement preview render for polarity and rotation.** Do not skip this.
7. Confirm live stock on the BOM.
8. Copy the exact files I ordered into `fabrication/revA/`, git commit and push.

Step 8 matters more than it looks. When Rev B happens, or when a board comes back wrong, the only way to know what I actually ordered is to have frozen the files I actually sent.
