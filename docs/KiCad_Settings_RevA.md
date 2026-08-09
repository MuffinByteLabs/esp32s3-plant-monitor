# KiCad Settings Log — ESP32-S3 Plant Monitor Rev A

*Record of every custom setting configured before layout, with the reasoning. Started 2026-08-04 (pre-layout setup). KiCad 10, **4-layer** (was 2 — changed 2026-08-06, see §5), JLCPCB. Update this file whenever a setting changes — it is the "why" behind the numbers in `ESP32S3_PlantMonitor.kicad_pro`.*

---

## 1. Library housekeeping

- `hardware/fp-lib-table` de-duplicated — `PlantMonitor_JLC` was listed 4× (each footprint import appended a copy). Now one entry + `logos`. Duplicates cause "duplicate nickname" warnings and confuse footprint resolution.

## 2. Board Setup → Design Rules → Constraints

| Field | Value | Why |
|---|---|---|
| Minimum clearance | **0.2 mm** | DRC floor for copper spacing. JLC can do 0.127; 0.2 ≈ 1.5–2× cushion. |
| Minimum track width | **0.2 mm** | Same logic. Signals route at 0.2–0.25. |
| Minimum connection width | **0.2 mm** | Catches accidentally-necked zone connections (thin copper acts like a fuse). |
| Minimum annular width | **0.15 mm** | JLC ring minimum; the 0.6/0.3 via has exactly 0.15. Ring = insurance against drill wander. |
| Minimum via diameter | **0.5 mm** | Working via is 0.6 — margin below it. |
| Copper to hole clearance | **0.3 mm** | JLC: PTH hole to track ≥ 0.28 (0.35 recommended). Was 0.25, bumped 2026-08-04. |
| Copper to edge clearance | **0.5 mm** | JLC needs ≥ 0.2 from routed edges (router bit wanders ±0.2). |
| Minimum drill size | **0.2 mm** | Deliberately below the usual 0.3: the ESP32 module footprint carries 12 × Ø0.2 thermal vias in its center pad. JLC 2-layer allows drills to 0.15; per their surcharge rule (extra cost only when a 0.2/0.25 hole has pad < 0.45) the 0.6 pads are surcharge-free. Own routing vias stay ≥ 0.3. |
| Hole to hole clearance | **0.5 mm** | JLC wants 0.45 between pad holes; closest real pair on this board is 0.7 (module via grid). |
| Silk min text height / thickness | **1.0 / 0.15 mm** | JLC legibility floor — smaller prints as mush. |
| uVia fields, arc deviation, zone-fill strategy, length tuning | defaults | Microvias aren't used on JLC's standard 4-layer process either (that's HDI, extra cost) — every via here is a plain through-hole via spanning F.Cu→B.Cu. Rest fine as-is. |

## 3. Board Setup → Design Rules → Pre-defined Sizes

- Tracks: **0.2 / 0.25 / 0.3 / 0.5 / 0.8 / 1.0 mm** (cycle with `W` while routing)
- Vias: **0.6/0.3** (standard) and **0.8/0.4** (power stitching)

## 4. Board Setup → Design Rules → Net Classes

| Class | Clearance | Track | Via | DP width / gap | Assigned nets (patterns) |
|---|---|---|---|---|---|
| Default | 0.2 | 0.2 | 0.6/0.3 | 0.2 / 0.25 (unused) | everything not listed below |
| **Power** | 0.2 | **0.5** | **0.8/0.4** | — | `+3V3` · `+5V_PROT` · `VSYS` · `VBAT` · `*VBAT_RAW` · `*USB_VBUS` |
| **USB** | 0.2 | 0.25 | 0.6/0.3 | **0.25 / 0.2** | `USB_DP` · `USB_DN` |

The `*` wildcard on `*VBAT_RAW` / `*USB_VBUS` exists because those two are **local labels**, so their full net names carry a sheet prefix (`/07_Battery_PowerPath/VBAT_RAW`). All the bare names are global labels / power symbols. If any net is later converted to a hierarchical label, its name gains a sheet prefix → prepend `*` to that pattern and re-check the "Nets matching" preview in Board Setup.

## 5. Other Board Setup pages

- **Physical Stackup: 4 copper layers / 1.6 mm total** — changed from 2 → 4 on **2026-08-06**, before a single trace was routed. JLC's standard 1.6 mm 4-layer construction (`JLC04161H-7628`):

  | Layer | Material | Thickness | Er |
  |---|---|---|---|
  | F.Cu | copper, 1 oz | 0.035 mm | — |
  | dielectric 1 | prepreg 7628 | 0.2104 mm | 4.4 |
  | In1.Cu | copper, 0.5 oz | 0.0152 mm | — |
  | dielectric 2 | core, FR4 | 1.065 mm | 4.6 |
  | In2.Cu | copper, 0.5 oz | 0.0152 mm | — |
  | dielectric 3 | prepreg 7628 | 0.2104 mm | 4.4 |
  | B.Cu | copper, 1 oz | 0.035 mm | — |

  Er values are now filled in rather than left "not specified" — they were cosmetic on 2-layer but they feed KiCad's impedance and length-tuning calculators, which matter once the USB pair gets real numbers.

  **Why 4 layers.** Not a speed requirement: the fastest signal on this board is USB **Full Speed** (12 Mb/s; ~5 ns edges ≈ 75 cm of physical edge length against a ~3 cm trace — nowhere near the trace > edge/10 threshold), and everything else (I²C 400 kHz, ADC, power) is slower still. A disciplined 2-layer board would have worked. Four layers were bought for **margin on a first layout**:
  - In1.Cu is a ground plane that *cannot* be cut, because nothing is ever routed on it. Hard rules 6, 19, 20 and 21 stop being constant vigilance and become automatic. On 2-layer, "will this via trench a return path?" is the judgement call a first-timer is most likely to get wrong, in the busiest part of the board.
  - Ground sits **0.21 mm** under F.Cu instead of 1.51 mm — ~7× closer, so signal-return loop area (and its inductance) drops by roughly the same factor, and a genuine 90 Ω USB pair becomes achievable instead of "close enough".
  - U2's ~0.34 W gets two more full copper sheets of heatsink (hard rule 15), which also lowers the local hot spot the BME280 is trying not to read (hard rule 17).
  - Slack. The subtle failure of a cramped 2-layer board isn't that it fails, it's the small ugly compromises made at ~80 % routed. Defects live there.

  Cost delta ≈ **$5** on five boards (JLC 4-layer board charge ~$70/m²; ~$2 → ~$7–8 at 78 × 70 mm); JLC **assembly** cost is identical either way, so it's a rounding error on the order.

- **Layer plan** (decided with the stackup):
  - **F.Cu** — every part (hard rule 22) + all fast or noise-sensitive signals: USB D+/D−, ADC_SOIL, BAT_SENSE, the VSYS node.
  - **In1.Cu** — one solid GND zone covering the whole board. Never routed on. Never cut. Draw it once, then leave it alone.
  - **In2.Cu** — power zones: `+3V3` as the main pour, with `VSYS` / `VBAT` / `+5V_PROT` islands.
  - **B.Cu** — slow signals only (I²C, UART to TP11/TP12, LED/button, EN, IO0) + a GND pour in the leftovers.
  - Rationale for the F.Cu/B.Cu split: B.Cu's nearest plane is In2, which is chopped into power islands, so its return paths are messier than F.Cu's — which sits over uncut ground. Anything speed- or noise-sensitive stays on top.
- **Antenna keep-out is now a 4-layer rule area** — Rule Area with "keep out copper pours" ticked on F.Cu, In1.Cu, In2.Cu *and* B.Cu. Hard rule 1's intent is unchanged; there are just two more layers to enforce it on, and the two new ones are invisible, so it is easy to forget.
- **Text & Graphics → Defaults:** silk text 1.0 mm height / 0.15 mm line width (matches constraint + JLC floor).
- **Solder Mask/Paste:** zeros — JLC applies their own mask expansion.
- **Teardrops / Violation Severity:** defaults.

## 6. Interactive router (Route → Interactive Router Settings)

Mode **Shove** (pushes existing tracks/vias aside while keeping every clearance rule satisfied — correct-by-construction). Shove vias ✓ · Jump over obstacles ☐ · Remove redundant tracks ✓ · Optimize pad connections ✓ · Smooth dragged segments ✓ · Optimize entire track ☐ (personal taste) · Mouse-path posture ✓ · Fix all segments on click ✓.

## 7. Grids (Preferences → PCB Editor → Grids)

- Fast Grid 1 (**Alt+1**): **0.5 mm** — part placement (things align by construction)
- Fast Grid 2 (**Alt+2**): **0.25 mm** — fine nudging; custom entry added to the grid list. 0.25 nests inside 0.5 so fine-tuned parts stay aligned with coarse-placed ones.
- Grid Overrides: all unchecked.

## 8. Annotation conventions (non-fab, personal)

- Floorplan boxes + notes live on **`User.Comments`** — one box per schematic sheet territory (02 USB / 03 3V3 / 04 Core / 05 Sensors / 06 Soil / 07 Battery / 08 Mech). Never on Silk (would print) or Edge.Cuts (would cut).
- Annotation color: **#00FF7F spring green** (custom color theme copy — built-in themes are read-only; Preferences → PCB Editor → Colors → New Theme). Green has no meaning in the board view (red = F.Cu, blue = B.Cu, yellow = silk, pink = courtyard, white = ratsnest), so it always reads as "note, not board."
- Sheet territories may be **grouped** (right-click → Group Items) so a whole cluster moves as one; double-click enters a group.

## 9. Planned-but-not-yet-applied (set these when the step arrives)

- **In1.Cu GND zone — draw this FIRST, before routing.** One rectangle over the whole board, net `GND`, so the plane exists while I route and I can see what I'm referencing. Clearance 0.3 / min width 0.2 / thermal spoke 0.5.
- **In2.Cu power zones** — after the power parts are placed: `+3V3` main pour, `VSYS` / `VBAT` / `+5V_PROT` islands. Zone priorities matter when islands overlap (higher priority wins).
- **F.Cu / B.Cu GND pours** — after routing, as before. Refill with `B` after *every* edit — stale pours lie.
- Any GND pad still showing an airwire after the pour → via next to it (stitching pass; extra vias around USB corridor, LDO pour, module EP). On 4-layer these vias now reach a plane that is 0.21 mm away instead of 1.51 mm, so they are cheaper electrically — use more of them, not fewer.
- Outline trim + corner fillets at the very end, before final DRC.

## 10. Related files

- `docs/Layout_Readiness_and_Placement_Guide_RevA.html` — footprint verification + placement/routing guide (2026-08-02)
- `references/JLCPCB_Capabilities_2026-08.md` — fab limits these settings are derived from
- `docs/PROJECT_STATUS.md` — overall project state
