# KiCad Settings Log — ESP32-S3 Plant Monitor Rev A

*Record of every custom setting configured before layout, with the reasoning. Started 2026-08-04 (pre-layout setup). KiCad 10, 2-layer, JLCPCB. Update this file whenever a setting changes — it is the "why" behind the numbers in `ESP32S3_PlantMonitor.kicad_pro`.*

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
| uVia fields, arc deviation, zone-fill strategy, length tuning | defaults | Microvias don't exist on 2-layer; rest fine as-is. |

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

- **Physical Stackup:** 2 copper layers / 1.6 mm total (0.035 Cu + 1.51 core + 0.035 Cu + masks) — standard JLC, unchanged. "Not specified" material fields left alone (cosmetic; JLC uses their own stock).
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

- **GND zones** (after routing, both layers): clearance 0.3 / min width 0.2 / thermal spoke 0.5. Refill with `B` after *every* edit — stale pours lie.
- Any GND pad still showing an airwire after the pour → via next to it (stitching pass; extra vias around USB corridor, LDO pour, module EP).
- Outline trim + corner fillets at the very end, before final DRC.

## 10. Related files

- `docs/Layout_Readiness_and_Placement_Guide_RevA.html` — footprint verification + placement/routing guide (2026-08-02)
- `references/JLCPCB_Capabilities_2026-08.md` — fab limits these settings are derived from
- `docs/PROJECT_STATUS.md` — overall project state
