# Finishing-Stage Review — Rev A

*Machine audit of `hardware/ESP32S3_PlantMonitor.kicad_pcb` as saved 2026-08-16, cross-checked against `Hard_Rules_Layout_RevA.md`, `KiCad_Settings_RevA.md`, `Finishing_Guide_RevA.md` and `PROJECT_STATUS.md`. Board state at audit: pours drawn (TOP_GND/BTM_GND), 140 vias placed, 394 track segments, silkscreen pass not started.*

---

## 1. Must fix

### 1.1 VBAT via pair violates hole-to-hole (DRC error)

Two `VBAT` vias sit at **(59.300, 80.400)** and **(60.175, 80.400)**.

| measure | value | rule |
|---|---|---|
| centre-to-centre | 0.875 mm | — |
| hole edge-to-edge | **0.475 mm** | `min_hole_to_hole` = 0.5 mm ✗ |
| copper edge-to-edge | 0.075 mm | same net, so no clearance rule applies |

Minimum legal centre spacing is 0.900 mm, so this misses by 0.025 mm. It is manufacturable (JLC's own floor is 0.45 mm) but it fails my own rule and will show as a DRC error.

**Fix:** drag one via to ~1.2 mm centre spacing. At 0.075 mm copper gap the two pads have essentially merged into one blob anyway, so the doubling is currently buying me very little.

### 1.2 Test points and mounting holes are not excluded from the position file

All 12 test points and H1–H4 carry `exclude_from_bom` but **not** `exclude_from_pos_files`:

```
TP1..TP12   attr = exclude_from_bom
H1..H4      attr = exclude_from_bom
```

`Finishing_Guide_RevA.md` Phase 5 step 3 requires these off **both** the BOM and the placement file. As saved, 16 phantom entries will be written into the CPL.

**Fix:** select all 12 TPs + H1–H4 → footprint properties → tick **Exclude from position files**.

### 1.3 C3/C4 are DNP but not excluded from BOM or placement

```
C3   attr = smd dnp
C4   attr = smd dnp
```

This is the exact item flagged in `PROJECT_STATUS.md` § Standing facts ("C3/C4 carry `dnp` but not exclude-from-BOM"). Rather than relying on the JLC plugin's DNP handling at upload time, set it explicitly.

**Fix:** tick **Exclude from bill of materials** and **Exclude from position files** on both.

---

## 2. Should fix

### 2.1 Power-IC ground pads are on thermal relief

| pad | part | size | zone connection | nearest GND via |
|---|---|---|---|---|
| U2 pad 2 | AP2112K-3.3 LDO, SOT-23-5 | 1.325 × 0.6 | inherit → thermal relief | 1.56 mm |
| U6 pad 2 | MCP73831 charger, SOT-23-5 | 1.325 × 0.6 | inherit → thermal relief | 2.08 mm |

Both are parts whose GND pin is their only heatsink (LAW 15 says so explicitly for U2; the charger dissipates on the same principle). Both are reflowed by JLC, so the hand-solderability rationale for thermal relief does not apply to either.

**Fix:** double-click each pad → **Connection to copper zones → Solid**. Same treatment resolves the two open DRC thermal errors on U4 pad 1 and U3 pad 40.

### 2.2 Outer pours still have KiCad's default thermal gap

| zone | layer | clearance | thermal gap | spoke |
|---|---|---|---|---|
| TOP_GND | F.Cu | 0.3 | **0.5** | 0.5 |
| BTM_GND | B.Cu | 0.3 | **0.5** | 0.5 |
| GND1_PLANE | In1.Cu | 0.3 | 0.3 | 0.5 |
| GND2_PLANE | In2.Cu | 0.3 | 0.3 | 0.5 |

Clearance was set to 0.3 but the thermal gap kept the stock 0.5. Bringing both outer zones to 0.3 matches the inner planes and improves the odds of a second spoke resolving on cramped pads.

### 2.3 Perimeter and corner stitching incomplete

5 % of the outer pour area sits more than 5 mm from any GND via (LAW 21). Worst regions:

| location | nearest GND via |
|---|---|
| (35, 92) bottom-left | 8.06 mm |
| (95, 52) top-right | 7.57 mm |
| (77, 74) right-centre | 7.54 mm |
| (35, 76) left edge near J1 | 7.32 mm |
| (74, 78) | 6.54 mm |
| (94, 93) bottom-right | 6.25 mm |
| (78, 65) | 5.88 mm |
| (36, 52) top-left | 5.80 mm |

Consistent with the perimeter pass (Phase 2 item 5) not being done yet. Corners are constrained by H1–H4; the open edges are the right side (x ≈ 95.5 between the mounting holes) and the bottom (y ≈ 93.5).

### 2.4 TP7 has no ground via

TP7 is a GND test point — a scope ground reference during bring-up — with no track and no via; its nearest GND via is **3.33 mm** away and it reaches ground only through thermal-relief spokes. Give it its own via.

### 2.5 One un-doubled power via

| net | vias | hops | singles |
|---|---|---|---|
| +3V3 | 10 | 5 | 0 ✓ |
| VBAT | 4 | 2 | 0 ✓ |
| +5V_PROT | 4 | 2 | 0 ✓ |
| USB_VBUS | 5 | 3 | **1 at (46.4, 74.0)** |

The "three single-via +3V3 hops" noted as open in the Finishing Guide are now all doubled. One VBUS hop remains single. Not a capacity problem — a 0.4 mm drill via carries well over the ~1 A involved — purely a consistency item.

### 2.6 Silkscreen pass not started

Zero standalone silkscreen text objects on the board. Phase 4 of the Finishing Guide is entirely outstanding: TP function labels, the J3 `+`/`−` safety marks (LAW 27), pin-1 marks at J2/J3, `BOOT`/`RESET` at SW1/SW2, `PWR`/`CHG` at D2/D3, board name and rev, and the `JLCJLCJLCJLC` order-number placement.

---

## 3. Verified clean — no action

- **Inner planes untouched (LAW 19/20).** Zero track segments on In1.Cu or In2.Cu. 394 segments split 308 F.Cu / 86 B.Cu.
- **No orphaned copper.** TOP_GND fills as 5 regions (1585, 152, 67, 5.5, 1.1 mm²) — every one contains at least one GND via. BTM_GND, GND1_PLANE and GND2_PLANE each fill as a single region.
- **Track widths match netclasses.** Power nets uniformly 0.5 mm (≈1.4 A capacity at 10 °C rise vs ≤0.5 A demand); Default-class signals at 0.2 mm; USB pair at 0.25/0.29 mm. No necked segments.
- **LAW 12 satisfied.** Every one of the 18 capacitors has a GND via 1.1–1.45 mm from its ground pad — i.e. immediately beside the pad.
- **LAW 15 satisfied.** U2's ground region carries 5 GND vias within 3 mm.
- **R13's ground via is done** — 0.8 mm track from the pad to a via at (54.0, 87.375). This open item from the Finishing Guide can be closed.
- **All 12 test point nets correct** against the bring-up probe table (TP1 +5V_PROT, TP2 VSYS, TP3 +3V3, TP4 VBAT, TP5 BAT_SENSE, TP6 ADC_SOIL, TP7/TP8 GND, TP9 EN, TP10 BOOT, TP11 TXD0, TP12 RXD0).
- **LAW 18 satisfied.** ADC_SOIL and BAT_SENSE stay 21.6 mm from the USB pair.
- **All 51 fitted parts carry LCSC part numbers.** No gaps.
- **Via inventory clean.** 140 vias, only the two predefined sizes (96 × 0.6/0.3, 44 × 0.8/0.4). No via violates copper-to-edge (0.5 mm), none sits outside the outline or inside a corner arc, and no via-to-THT-hole pair is under 0.5 mm.
- **Antenna keep-out rule area still absent** (0 rule areas in the file), consistent with the conscious waiver in PROJECT_STATUS. With the antenna fully overhanging and the pour clipped 0.5 mm from the edge, no copper can reach under it.

---

## 4. Observations — judgement calls, not defects

**Only three test points are now inside the 15 mm antenna ring, not four.** Measured to the antenna keep-out rectangle:

| TP | distance |
|---|---|
| TP5 | 8.0 mm |
| TP3 | 10.4 mm |
| TP9 | 12.0 mm |
| TP7 | **15.2 mm — now outside** |

TP7 has moved out since the 08-15 status note. The open decision is now smaller than recorded.

**ADC_SOIL passes within 2.3 mm of the antenna keep-out rectangle** (BAT_SENSE, 3.5 mm). Strictly this is tighter than LAW 4's 15 mm, but it is forced by the pinout — the module's ADC pin (pad 39) is physically at the top edge beside the antenna, and LAW 18's non-negotiable requirement (C14/C17 sited at the pin) depends on staying there. The trace is a microstrip over solid ground 0.21 mm below, which is a fundamentally different coupling case from the free cable LAW 4 was written about — and the actual soil cable exit at J2 is 38.5 mm away, which is what the rule cares about.

---

## 5. Suggested order of work

1. Move the VBAT via (§1.1) — 30 seconds, clears a DRC error.
2. Set U2/U6/U4/U3 ground pads to solid (§2.1) — clears the two thermal errors legitimately.
3. Drop TOP_GND/BTM_GND thermal gap to 0.3 (§2.2), refill, re-run DRC.
4. Add the TP7 via (§2.4).
5. Perimeter stitching pass (§2.3).
6. Tick the exclusion flags on TPs, H1–H4, C3, C4 (§1.2, §1.3).
7. Silkscreen pass (§2.6), then the DRC loop and the JLC upload gauntlet.

---

*Generated by machine audit of the saved board file. Every figure above was read from `ESP32S3_PlantMonitor.kicad_pcb`; nothing is inferred from the design documents alone. Companion to `docs/Finishing_Guide_RevA.md`.*
