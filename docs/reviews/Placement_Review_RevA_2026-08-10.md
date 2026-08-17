# Placement Review — ESP32-S3 Plant Monitor Rev A
*Deep placement check, 2026-08-10, from the saved board file (post-placement, pre-routing: 69 footprints, no tracks/zones yet). Everything below was measured programmatically from `ESP32S3_PlantMonitor.kicad_pcb` — every pad coordinate, courtyard, and net — and checked against `docs/Hard_Rules_Layout_RevA.md` (LAW/STRONG-PRACTICE numbering used below), the Layout Readiness guide, Espressif's hardware design guidelines, and general IPC-style practice. Companion picture: `Placement_Review_RevA_2026-08-10_map.png`.*

**Verdict: this is a strong placement.** The architecture is exactly right — protection in copper order at the USB connector, flow-through ESD, every decoupler measured at 1.4–3.7 mm from its pin, antenna fully off-board with an empty keep-out, sensors in the cool corner, single-sided assembly, zero courtyard collisions. Four things should change before I route, and none of them is painful today: **pull both JST connectors in (~3 mm), fix the J1 overhang by 0.46 mm, round the outline corners, and patch the USB net-class patterns.** Test-point positions are proposed below with verified coordinates.

---

## 1 · What was checked and passed

**Decoupling & critical proximity (LAW 11, 33, 36 — pad-to-pad, mm):**

| Part → pin | mm | Part → pin | mm |
|---|---|---|---|
| C10 100n → U3 pin 2 (3V3) | **1.86** | C15 → U6 VDD | **1.75** |
| C9 22µ → U3 pin 2 | 3.66 | C16 → U6 VBAT | **1.76** |
| C6 1µ → U2 VIN | **1.84** | C11 → BME VDD | **1.49** |
| C5 10µ → U2 VIN | 2.98 | C12 → BME VDDIO | **1.57** |
| C7 1µ → U2 VOUT | **1.86** | C13 → VEML VDD | **1.80** |
| C14 → U3 pin 39 (ADC_SOIL) | **2.05** | C8 → U3 EN | **1.92** |
| C17 → U3 pin 38 (BAT_SENSE) | **2.12** | R6 → U3 IO0 | **1.89** |
| C1 → U1 VBUS | **1.84** | R16 → Q3 gate | **1.51** |
| R1/R2 → U3 pins 13/14 | **1.7** | R13 → U6 PROG | **1.67** |

Smallest-cap-closest ordering is correct in both places it applies: supply flows C9(22µ)→C10(100n)→pin 2, and C5(10µ)→C6(1µ)→VIN (STRONG 33). C14/C17 sit at the module ADC pins exactly as the corrected LAW 18 demands, with R11 and the R14/R15 divider riding alongside.

**USB chain (LAW 5–10):** copper order is physically enforceable as placed — J1 VBUS pads (x=40.2) → D1 TVS (2.65 mm) → F1 (2.1 mm further) → everything else. U1 is a true flow-through: pins 1/3 face J1 carrying the connector-side nets, pins 6/4 exit to R1/R2, which sit 1.7 mm from the module's USB pins. C3/C4 (DNP) hang on the pair with millimetre stubs. R3/R4 are in the cluster with short CC drops. The J1→U1 corridor at y≈77.5 is straight and clear.

**Radio (LAW 1–4):** the entire antenna section (module y 43.5–49.8) overhangs the top edge — Espressif's preferred option — and the keep-out contains nothing on any layer. All three off-board exits (USB left, soil right, battery bottom) face away; measured cable-exit distances to the antenna zone: J1 ≈ 34 mm, J2 ≈ 29 mm, J3 ≈ 41 mm — all ≥ 15 mm with margin. The UART trio TP8/11/12 sits 43 mm away at the bottom edge, clustered at 2.54 mm pitch (header-compatible — nice touch).

**Thermal (LAW 15–17):** BME280 to LDO 48.7 mm, to module 33.5 mm, to charger 30.3 mm — the coolest quadrant available. VEML7700 is 34 mm from the LEDs.

**Power path (LAW 13):** VSYS node (Q3 source, D4 cathode, C18/C5/C6, U2 VIN) spans just 7.8 × 2.5 mm. Q2 sits 3.1 mm from J3 pin 1. Battery hand-over cluster is compact.

**Assembly & mech (LAW 22–26):** all 53 parts + 12 TPs on F.Cu (single-sided ✓); zero courtyard overlaps; all rotations 0/90/180; H1–H4 screw-head zones (Ø6) are part-free; buttons same orientation, 8.25 mm apart; connector openings all face off-board.

---

## 2 · Changes to make before routing (prioritized)

### 2.1 HIGH — J2 and J3 housings hang ~3 mm off the board

Both JST PH side-entry housings extend well past the edge: the S3B/S2B body runs 6.25 mm forward of its pin row, so:

| | pin row | body front | board edge | overhang |
|---|---|---|---|---|
| J2 (soil) | x=97.55 | x=103.80 | x=100.25 | **3.55 mm** |
| J3 (battery) | y=91.30 | y=97.55 | y=94.50 | **3.05 mm** |

Standard mounting (JST drawing, KiCad footprint intent, IPC practice) is the housing fully on the board with the mating face flush to ~1 mm proud. At 3+ mm, half the housing floats: every insertion (~10–15 N for PH) levers the unsupported front against the solder joints, and the courtyard hanging off-board will confuse any future enclosure/3D work. It won't fail tomorrow — THT pins are strong — but it's the one genuinely non-standard thing on the board, and both connectors are off by the same amount, which suggests the footprint's origin (pin 1, not body center) was placed to the edge distance intended for the face.

**Verified fix (no other parts move):**
- **J2: (97.55, 78.5) → (94.5, 72.0)**, same rotation. Face lands at x=100.75 (0.5 mm proud). Slides up the edge to clear Q1's courtyard by 0.6 mm; R10/Q1 stay put; the ADC_SOIL run to C14 gets ~6 mm shorter as a bonus. Soil-cable exit remains ≈29 mm from the antenna zone (LAW 4 ✓).
- **J3: (64.0, 91.3) → (64.0, 88.75)**. Face lands at y=95.0 (0.5 mm proud).
- **Q2: (63.75, 87.25) → (59.5, 89.5)**, keep rot −90°. It must move because J3 comes up into its space; this spot keeps it 4.8 mm from J3 pin 1 (VBAT_RAW entry, reverse-protection still first in line) with clean runs up to C16/U6, and clears U6, TP8, and J3's new courtyard (0.35 mm+).

All three positions were collision-checked against every courtyard on the board.

*(Alternative if I prefer J2 at its current height: J2 → (94.5, 78.5) forces Q1 → ~(93.0, 82.9) and a cramped R10 relocation — more churn, no benefit. The slide-up version is cleaner.)*

### 2.2 MEDIUM — J1 nose overhang is 1.46 mm; my LAW 10 target is ~1.0 mm

Measured: shell nose at x=32.54 vs edge x=34.00 → **1.46 mm** overhang. The HRO drawing's rule (edge 1.6 mm from the shield-slot pair at x=35.14) puts the edge at x=33.54, which is exactly a 1.00 mm nose overhang. I'm 0.46 mm over — the plug will still seat (extra overhang fails safe; recessed is the dangerous direction), but LAW 10 says "exactly per the drawing," and enclosure flushness assumes the spec value.

**Cleanest fix: move the left board edge, not the connector.** Change the outline rect's left side from x=34.00 → **x=33.54** (board grows 0.46 mm; H1/H3 and copper-edge clearances all still pass — verified). Moving J1 +0.46 mm instead cascades: its courtyard then hits U1 (0.43 mm) and D1 (0.28 mm), so J1+U1+C1+D1 would all shift. One number in the outline beats four part moves.

### 2.3 MEDIUM — USB pair segments are anonymous nets, so the router treats them as `Default`

*(Revised 2026-08-12 — the schematic-side fix below supersedes the net-class-pattern patch originally written here; my own suggestion.)*

The `USB` class (0.25 mm track, 0.2 mm diff-pair gap — correctly configured) is assigned only to `USB_DP`/`USB_DN`, which exist **only from R1/R2 to the module** (2.5 mm). The long stretch I'll actually route as a pair — J1 → U1 → R1/R2 — lives on auto-generated names (`Net-(J1-D+-PadA6)`, `Net-(J1-D--PadA7)`, `Net-(R1-Pad1)`, `Net-(R2-Pad1)`), which fall into `Default` (0.2 mm, 0.25 dp-gap). Worse than the width: KiCad's differential-pair router finds a net's partner **by name suffix** (…P/…N, +/−), and `Net-(J1-D+-PadA6)`/`Net-(J1-D--PadA7)` don't form a recognizable pair — so `Route → Differential Pair` won't even engage on 80 % of the run.

**Fix (industry-standard: name the nets in the schematic — the schematic is the source of truth, not the layout tool's patterns):** on sheet 02, add net labels to the four unlabeled wire segments, keeping the P/N-at-the-end convention so the pair router recognizes them:

| Segment | Label |
|---|---|
| J1 A6/B6 (D+) → U1 pin 3 | `USB_CONN_DP` |
| J1 A7/B7 (D−) → U1 pin 1 | `USB_CONN_DN` |
| U1 pin 4 → R2 | `USB_ESD_DP` |
| U1 pin 6 → R1 | `USB_ESD_DN` |

Then: ERC (stays clean) → **F8** to push the names to the board — zero-risk right now because nothing is routed; pads just get renamed nets — and in Board Setup → Net Classes add two patterns to `USB`: `*USB_CONN_D*` and `*USB_ESD_D*` (leading `*` because sheet-local labels carry the `/02_USB_C_Input/` path prefix — same trick my Power class already uses for `*USB_VBUS`). Now every segment routes as a true differential pair at 0.25/0.2, DRC messages read `USB_CONN_DP` instead of pad soup, and the stage-by-stage names document the signal flow (connector → ESD → series-R → module) the way commercial schematics do.

### 2.4 LOW-MED — outline is a sharp-cornered rectangle

My own guide's step 2 called for a 1–2 mm corner radius. Rounded corners handle better, don't chip, and are kinder to enclosures and pockets. Add **R2 mm** arcs at the four corners (clears all mounting holes comfortably). Do it now — after routing, edge-clearance ripples make it annoying. If I change the left edge per 2.2, draw the radius after.

### 2.5 LOW — lock the edge-committed parts

Guide step 3 says lock J1 and U3 once set; neither is locked in the file (no `locked` attributes present). After the J1/edge decision, select J1 and U3 → `L`. Worth locking J2/J3 after the 2.1 moves too.

### 2.6 Watch items — no action needed, just know

- **Razor-thin courtyard gaps** (fine today, zero slack): R11–R15 and C10–C8 at **0.015 mm**, U1–J1 at 0.03, C13–R8 / C3–R1 / C4–R2 / R14–R15 / C10–C9 at 0.04, C10/C8/C3/C4/R1/R2 to the module courtyard at 0.05. Any nudge of these parts trips DRC — move them only deliberately.
- **U3's top GND pads (pins 1/40) sit 0.54 mm from the board edge** — passes my 0.5 mm copper-edge rule by 0.04 mm. Dictated by the antenna overhang; nothing to do, just don't be surprised the DRC margin is thin there.
- **H2's screw sits 17.5 mm laterally from the antenna zone** — meets the ≥15 mm enclosure-metal rule, but without much margin. Prefer nylon standoffs at H2 (top-right) when the case happens.
- **C3/C4 carry the `dnp` attribute but not "exclude from BOM/pos"** — my BOM was generated correctly by hand; just re-confirm at JLC upload that both stay off the BOM and placement files (my LAW 30 / pre-order step already covers this).
- Board is 66.25 × 44.5 mm vs the guide's ~52 × 42 suggestion — roomier is a fine call for a first board ("boards grow badly but shrink well"); Rev B can compact.

---

## 3 · Test-point placement plan (the 9 still parked off-board)

All positions collision-verified against every courtyard (≥0.3 mm clear except where noted), ≥2.5 mm between TPs, outside the four Ø6 screw-head zones, and ≥15 mm from the antenna zone per LAW 4 — that last rule is why TP3/TP7 are not tighter to the LDO: a probe wire clipped anywhere closer than 15 mm counts as an off-board conductor. TP8/11/12 stay exactly where I put them.

| TP | Net | Position (x, y) | Why there |
|---|---|---|---|
| TP1 | +5V_PROT | **(50.6, 82.5)** | Just downstream of F1/C2, on the natural VBUS→charger run |
| TP2 | VSYS | **(44.9, 65.3)** | Below the C5/C18 cluster — probes the hand-over node (scope here for the R16 body-diode notch, BringUp step 9) |
| TP3 | +3V3 | **(52.0, 62.5)** | On the LDO→module 3V3 trunk; 15.3 mm from antenna zone (any closer to U2 violates LAW 4) |
| TP7 | GND | **(53.8, 64.8)** | Scope/DMM ground beside TP3/TP2 — one ground reach for all three power rails |
| TP4 | VBAT | **(60.0, 80.5)** | Above the charger cluster, clear of Q2 in both its current and proposed positions |
| TP5 | BAT_SENSE | **(89.0, 62.5)** | Right of the divider; 16.1 mm from the antenna zone — dress probe leads down-right, not toward the module |
| TP6 | ADC_SOIL | **(91.0, 68.0)** | On the J2→C14 route (assumes J2 moves per 2.1; if J2 stays, use (92.8, 80.3)) |
| TP9 | EN | **(60.3, 72.8)** | Beside SW2 (RESET) — natural bench spot |
| TP10 | IO0 | **(73.2, 71.3)** | Between module bottom row and SW1 (BOOT); 0.3 mm to SW1 courtyard — placed deliberately, don't nudge |

Silk phase (LAW 27): label every TP with its **name** (+5V, VSYS, 3V3, VBAT, BSNS, SOIL, GND, EN, IO0, TX, RX, GND), not just the refdes.

---

## 4 · Routing prep (my next step after TPs)

Order of battle, per my guide §5/§6 with board-specific notes:

1. **Housekeeping first:** the USB net renames + F8 + class patterns (2.3), edge/corner decisions (2.2/2.4), lock parts (2.5).
2. **USB pair first** (least freedom): J1 pads → merge A/B pad pairs at the connector → straight through U1 (pins 1/3 in, 6/4 out) → diagonal corridor to R1/R2 at (58.5, 66–67.5) → module pins 13/14. Top layer only, zero vias, ~0.2 mm gap, and keep the bottom layer under the pair + 5 mm shadow untouched (LAW 5/6). The corridor passes below the LED cluster — route the pair before anything else claims that diagonal.
3. **VBUS/protection in copper order** (LAW 7): J1 VBUS pads → D1 → F1 → C2/onward, 0.5 mm+, stubs in millimetres — placement already makes the straight chain possible; don't route VBUS to F1 first and tee back to D1.
4. **Power spine:** F1 → (C2) → split: U6 VDD (via C15) and D4 → VSYS pour-let (D4/Q3/C18/C5/C6/U2·VIN fat and compact, 0.5–0.8 mm, LAW 13/14) → LDO → C7 → 3V3 trunk east along y≈53–54 past C9→C10 into pin 2, then a second 3V3 branch south to the sensor corner. Give U2's GND pin its heat-spreader pour + 4–6 stitching vias (LAW 15 — confirmed requirement from both prior reviews).
5. **Analog:** ADC_SOIL from J2 up the right side into C14/pin 39; BAT_SENSE tap from Q2/C16 up the right corridor to R14 — keep both off the top strip beside the module (x 60.8–78.8, y < 52.5), never alongside the USB pair, ground pour alongside (LAW 18).
6. **Ground discipline:** every decoupler GND pad gets its own via at the pad (LAW 12 — that's ~20 vias, place them as I route each cap); bottom layer stays one unbroken plane (LAW 19); stitch top pour to bottom every ~5 mm and ring the antenna keep-out with vias — around, never inside (LAW 3, 21).
7. **UART to the trio:** TXD0/RXD0 from pins 36/37 down the right side then west along the lower board to TP11/TP12 — length is free, just keep pour beside it (STRONG 34).
8. Pour both layers, DRC with the JLC rules, fix, re-pour, repeat; then the 3D sanity pass (connector overhang, antenna clear, button reach).

Pre-order gates stay as my §7 list: ERC/DRC clean, JLC preview rotation/polarity check on every polarized part, C3/C4 + TPs + holes excluded, live stock re-check, freeze into `fabrication/revA/`, commit and push.

---

*Measured from the board file, not screenshots · all coordinates in KiCad board units (mm, Y down).*
