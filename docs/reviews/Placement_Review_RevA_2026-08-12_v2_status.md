# Placement Re-Check — Rev A v2 (after implementing review changes)
*2026-08-12, measured from the saved board file. Companion picture: `Placement_Review_RevA_2026-08-12_v2_map.png`. Follows up on `Placement_Review_RevA_2026-08-10.md`.*

**Verdict: nearly everything landed, and the board got better than the review asked for.** I shrank the outline 3.75 mm (now 62.5 × 44.5), re-centered the module on the new edge, rounded the corners at R4.25 with the M3 holes concentric to them (uniform 1.25 mm edge margin around each screw head — tidier than my earlier R2 suggestion), and the full re-check of the new placement is clean: **zero courtyard overlaps, all four screw-head zones clear, every connector face correct except one, all decouplers still within their limits.** One item was over-corrected, one column is worth a small snug, and the test points remain to place — fresh verified coordinates below (the ones in the 08-10 report are stale; the floorplan moved under them).

## Scorecard vs. the 08-10 review

| Finding | Status |
|---|---|
| 2.1 J2 housing overhang (was +3.55 mm) | **FIXED** — face now +0.30 mm proud; Q1/R10 moved with it; cable exit 32.9 mm from antenna zone ✓ |
| 2.1 J3 housing overhang (was +3.05 mm) | **FIXED** — face +0.30 mm; Q2 relocated cleanly (J3→Q2→C16→U6 chain: 4.2 / 1.9 mm) ✓ |
| 2.2 J1 nose overhang | **OVER-CORRECTED** — see below |
| 2.3 USB net renames | **DONE** — `USB_CONN_DP/DN`, `USB_ESD_DP/DN` as local labels on sheet 02, F8 pushed to the board, `*USB_CONN_D*` / `*USB_ESD_D*` patterns added to the USB class. Diff-pair routing will now work end-to-end at 0.25 / 0.2. ✓ |
| 2.4 Corner radius | **DONE** — R4.25 arcs, holes concentric ✓ |
| 2.5 Lock J1/U3 | **PARTIAL** — H1–H4 locked (good, they're position-critical now); J1 and U3 still unlocked. Lock them after the J1 nudge below. |
| Test points | **PENDING** (expected) — new coordinates below |

## The one fix to make: J1 moved 1.0 mm, but the error was 0.46 mm

The nose overhang was 1.46 mm and needed to come down to ~1.0. Moving J1 from x=36.19 → 37.19 removed a full millimetre, so the nose now sits at 33.54 vs the edge at 34.00 = **0.46 mm overhang — under the HRO target**. This is the less-safe direction: too little overhang risks the plug shell bottoming on the board edge before the latch engages.

**Fix: J1 → x = 36.65** (0.54 mm left; y stays 77.50). That puts the nose at 33.00 = exactly 1.0 mm proud, and the shield slots land exactly 1.6 mm behind the edge per the drawing. Verified against the new positions of U1/C1/D1/R3: gaps only grow (D1 0.72 mm, U1 0.78 mm; R3 stays 0.03 mm vertically as before — untouched by an x-move). Then lock J1 and U3.

## Worth a small snug (optional): the module-right column lagged the module

U3 moved 3.89 mm left; the C14/C17/R11/R15/R14/R6 column moved only 2.03 mm, so their pin distances grew: C14→pin 39 now 3.87 mm (was 2.05), C17→pin 38 3.91 (was 2.12), R6→IO0 3.75 (was 1.89). Still inside my "millimetres not centimetres" LAW — nothing is wrong — but sliding the six-part column ~1.8 mm left restores the old figures and shortens my most delicate analog nets. Space is free there (nearest obstacle after the move is the module courtyard at ~0.25 mm, same relationship as v1). C9 (22 µF) similarly sits at 4.0 mm from pin 2 behind C10 at 1.96 — fine, big cap goes behind the small one anyway.

## Everything re-verified clean on v2

Zero courtyard overlaps · tightest gaps unchanged in character (R11–R15 0.015, R11–C14 / R14–R15 0.04, C14–C17 0.065, D1–R3 0.12 — don't nudge those clusters) · decouplers: C10 1.96, C6 1.85, C7 1.86, C15 1.86, C16 1.90, sensors 1.5–1.8, R1/R2 1.8, R16 1.75, R13 1.79 mm · VSYS node compact (~6 × 2.5 mm) · BME280 → LDO 45.4 / module 33.2 / charger 27.8 mm · VEML → LEDs 30.5 mm · antenna zone empty, module ~centered (65.89 vs 65.25) · H2 lateral to antenna zone 17.4 mm · J2/J3 cable exits 33/41 mm from the zone · all parts top-side, rotations 0/90/180 · label audit unchanged and correct (new USB names properly local).

## Test points — fresh verified coordinates for the v2 floorplan

Same rules as before (≥0.3 mm to every courtyard, ≥2.5 mm between TPs, outside Ø6 screw zones, ≥15 mm from the antenna zone, near their circuits). TP8/11/12 stay where they are.

| TP | Net | Position (x, y) | Notes |
|---|---|---|---|
| TP1 | +5V_PROT | **(51.5, 79.5)** | On the F1 → charger run; spur from F1 pad 2 ≈ 3 mm |
| TP2 | VSYS | **(43.4, 70.9)** | Below the C5/C6 input caps — scope here for the hand-over notch (BringUp step 9) |
| TP3 | +3V3 | **(47.8, 63.6)** | Beside the LDO output; 16.4 mm from antenna zone (closer violates LAW 4) |
| TP7 | GND | **(50.9, 64.2)** | 3.1 mm from TP3 — rail-probing pair; give it its own via to the plane |
| TP4 | VBAT | **(64.3, 80.8)** | Right of C16/Q2, clear of J3 |
| TP5 | BAT_SENSE | **(85.0, 62.0)** | 15.7 mm from the zone — dress probe leads down-right, away from the module |
| TP6 | ADC_SOIL | **(87.1, 71.6)** | On the J2 → C14 route, above J2's courtyard |
| TP9 | EN | **(57.9, 71.5)** | 1.9 mm from SW2's EN pad |
| TP10 | IO0 | **(66.4, 71.3)** | 1.9 mm from SW1's IO0 pad |

Silk labels by function (+5V, VSYS, 3V3, GND, VBAT, BSNS, SOIL, EN, IO0) when I do the silkscreen pass.

## After that

Type in the TPs, nudge J1, lock J1/U3, and I'm at the routing phase — the order in the 08-10 report §4 still applies verbatim (USB pair first through its corridor, VBUS in copper order, power spine, analog, per-cap ground vias, pours + stitching). The optional schematic niceties (+3V3 power-symbol unification, `SENS_PWR_EN` active-low rename, labels for EN/IO0/TXD0/RXD0) remain open and remain optional.

*all coordinates in KiCad board units (mm, Y down).*
