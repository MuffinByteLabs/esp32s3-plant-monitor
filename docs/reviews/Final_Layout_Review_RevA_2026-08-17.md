# Final Pre-Fab Layout Review — Rev A

*Independent machine audit of `hardware/ESP32S3_PlantMonitor.kicad_pcb` as saved 2026-08-17 (the snapshot taken this morning — 404 track segments, 154 vias, 4 zones, silkscreen pass done). Method: full KiCad 10.0.5 DRC with zone refill and schematic-parity, plus independent geometric analysis of every rule in `Hard_Rules_Layout_RevA.md` and the finishing checklist. Schematic assumed correct. Every number below was measured from the board file, not taken from the docs.*

**Verdict: this board is ready to manufacture after about 15 minutes of small fixes.** Copper, clearances, stackup, USB, power, decoupling, and assembly hygiene all check out — several items from the 08-16 finishing review are confirmed fixed (VBAT via spacing, thermal-gap 0.3, U2/U6/U4 solid pads, TP/H exclusions, C3/C4 exclusions, TP courtyards). What remains is two real fixes, one process sync, and a set of judgment calls I should close consciously rather than by default.

---

## 1. Fix before ordering

### 1.1 The two antenna-fence gaps are still open (from the 08-16 review §2.3)

The GND via fence along the top edge, measured on the current file (all vias y < 53.2, gaps center-to-center):

```
x:    36.6  41.75  45.5  48.9  50.88  53.88  58.5  61.1  64.08  67.08  70.08  72.1  77.83  84.5  87.5  94.2
gaps:   5.15   3.75   3.4   1.98   3.0    4.62  2.6   2.98   3.0    3.0    2.02   5.73   6.67   3.0   6.7
```

In the critical band beside/under the antenna (x ≈ 54–78) the same two gaps flagged on 08-16 remain: **4.62 mm** (53.88 → 58.5) and **5.73 mm** (72.1 → 77.83), against my ~3 mm (λ/20) target. The 08-16 prescription still applies verbatim: one via near **(56, 51.2)**, one near **(75.5, 51.0)** — the second lands on the existing 0.8 mm GND trace from U3 pad 40 to C14's ground. Two clicks, close it out. (The gaps further right, 77.8 → 84.5 → 87.5 → 94.2, are outside the antenna span — optional.)

### 1.2 J1's ground pads hang on single thermal spokes (3 DRC warnings)

The only genuine DRC findings on the whole board: **starved thermals on J1 pads A12, B1, and the rear-right shield slot** — each reaches the top pour through one 0.5 mm spoke instead of the required two. Related: the top pour fills two tiny scraps beside the connector (≈1.1 mm² at x 39.2–40.9 / y 82.1–83.5, and ≈0.6 mm² at x 40.7–42.1 / y 80.5–81.0) whose only anchor is those spokes — no via in either. Meanwhile the shield slots' nearest dedicated GND vias sit 4.4 mm away (the two front slots are fine at 1.4–1.8 mm).

My own LAW ("J1's shield holes and its GND pads take the cable yank — solid ties") wants better, and this connector is also the ESD entry point: shell → ground needs a low-inductance path, not one spoke into a floating scrap.

**Fix:** select J1's four GND pads (A1/B12, A12/B1) and the four SH slots → pad properties → **Connection to copper zones: Solid**. J1 is reflowed by JLC, so the hand-soldering rationale for reliefs doesn't apply. Then refill and, if either little scrap survives as a peninsula, drop one GND via into it (or let the solid connection absorb it). This clears all three warnings for the right reason.

### 1.3 Sync the C3/C4 exclusion flags into the schematic

The board footprints now carry *exclude from BOM / position files* (good — confirmed in the file), but the schematic symbols don't, which is exactly what the two remaining schematic-parity warnings say. Harmless for fabrication today, but my pre-order gauntlet ends with a final F8 — and an F8 with mismatched flags can silently re-import the attributes and resurrect C3/C4 into the BOM. Tick *Exclude from bill of materials* on C3/C4 in sheet 02, F8, and parity goes to zero. Two minutes, removes a booby trap from my own process.

---

## 2. Judgment calls to close consciously

### 2.1 Test points inside the 15 mm antenna ring — recommend: waive for Rev A

Current distances from the antenna keep-out rectangle (re-measured; TP7 confirmed outside since 08-16):

| TP | net | distance |
|---|---|---|
| TP5 | BAT_SENSE | 8.3 mm |
| TP3 | +3V3 | 10.6 mm |
| TP9 | EN | 12.5 mm |

This is my own conservative rule (Espressif's guidance is silent on test pads), the pads are 1.5 mm dots with no permanently attached wire, and the antenna fully overhangs the edge. The physics concern is the probe *lead* during RF-active bring-up, which is a bench discipline problem, not a layout problem. Moving TP3/TP5 buys little and risks disturbing a finished, DRC-clean top layer. **Waive it, write the waiver down, and keep probe leads off those three TPs during Wi-Fi tests** (my bring-up guide already implies this). If I ever spin Rev B, move TP5 south a few mm and the topic dies.

### 2.2 One un-doubled USB_VBUS via (from 08-16 §2.6)

Still there at **(46.40, 73.95)**. A 0.4 mm-drill via is good for well over the ~0.5 A this rail will ever see, so this is consistency, not capacity. Double it if I want a clean "every power hop doubled" claim (all other hops on +3V3 / +5V_PROT / VBAT / VBUS are doubled — verified); otherwise waive it in one sentence.

### 2.3 Stitching grid: six spots exceed my ~5 mm rule

Worst offenders (distance to nearest GND via): **(77.0, 74.5) — 7.6 mm**, (74.5, 87) — 6.1, (74.5, 80) — 6.1, (78, 65) — 5.9, (62.5, 76) — 5.8, (49, 76.5) — 5.2. All in the slow right-hand/sensor half of the board; nothing fast lives there, both planes are solid underneath, so the electrical value of filling these is small. Five or six pasted vias if I want LAW 21 strictly green; defensible to skip.

### 2.4 Charger ground could take one via

U6's GND pad is solid-connected to the pour (good, confirmed), but its nearest GND vias are 2–3 mm away (2 within 3 mm; the LDO by comparison has 5 within 3 mm). For ~0.2 W this works; one extra via beside the pad copper would match the LDO treatment. Optional.

### 2.5 Silkscreen polish (all cosmetic)

- Three labels carry a trailing newline — `5V\n`, `EN\n`, `+\n` (the J3 plus mark). With bottom-justified text the empty line shifts the visible glyphs up a line-height from where the anchor sits. They *render* where I placed them, so nothing is wrong today, but the stray `\n` makes them fragile to future nudges — worth cleaning while I'm in there.
- `SOIL_ADC` (vertical) and `SIG` start within ~0.5 mm of each other near J2. DRC found no actual stroke overlap and the render looks acceptable — just eyeball this corner in the JLC preview.
- No board name/rev text anywhere, and no `JLCJLCJLCJLC` token. Both were "optional flourishes" in my guide, but note the consequence of the missing token: **JLC will print their order number at a location of their choosing** unless I pay to remove it or place the token where I want it. On a board this tidy I'd place the token (bottom side, under J2/J3, is the natural spot — B.SilkS is empty).
- Refdes are hidden for 60 of 69 footprints (functional labels replace them — honestly nicer for this device, and JLC assembles from the CPL, not silk). Just keep the schematic open at bring-up. The 15 "silk clipped by edge" warnings are the overhanging J1/J2/J3/U3 housings — expected and correct.

### 2.6 Two micro-nits, for completeness

- The top-right corner arc is displaced **5.2 µm** left of true: the arc ends at x 96.494796 against the right edge at 96.5, leaving a 5 µm outline gap at (96.5, 54.25). KiCad chains it fine and JLC's CAM will heal it silently — but if I'm ever re-snapping that corner, drag the arc endpoints to (92.25, 50) / (96.5, 54.25).
- J1 and U3 were never locked (my own checklist item 4 from PROJECT_STATUS). Zero fab impact; lock them if I'm going to edit near them again.

---

## 3. Verified clean — the numbers behind the verdict

**DRC (KiCad 10.0.5, zones refilled, all severities, schematic parity on):** 0 errors, **0 unconnected items**, 0 copper/clearance/annular/edge/hole violations of any kind. Every warning is accounted for above. (The 65 "library" items in this run are an artifact of auditing outside my usual setup — my fp-lib-table resolves them.)

**Stackup & fab match:** file contains exactly JLC04161H-7628 — 0.035 / 0.2104 (εr 4.4) / 0.0152 / 1.065 (εr 4.6) / 0.0152 / 0.2104 / 0.035, total 1.6062 mm. 4-layer · 1.6 mm · green · Economic-PCBA-eligible. Track/space floor on the board is 0.2/0.2 vs JLC's 0.09/0.09 multilayer capability; copper-to-edge min measured 0.59 mm (tracks), 0.60 (vias), 0.501 (all four zone fills) against my 0.5 rule; 154 vias, only the two sanctioned sizes (0.6/0.3, 0.8/0.4), all tented both sides, none in pads; module EP's 12 × Ø0.2 drills sit in 0.6 pads — no JLC surcharge; NPTH pegs Ø0.65 and mounting holes Ø3.2 ≥ their 0.5 floor; shield slots 0.6 × 1.7 ≥ 0.5 wide plated-slot minimum; hole-to-hole all clear (the 08-16 VBAT via pair is fixed — now doubled at legal spacing).

**USB (LAW 5/6/7):** the D+/D− pair runs **entirely on F.Cu with zero vias**, J1 → U1 (flow-through) → R1/R2 → module pads 13/14, in copper order. Coupled run at 0.29 width / 0.2 gap (0.49–0.50 center-to-center measured everywhere it's parallel) — that's the ~90 Ω geometry for this stackup, better than the plan. Length mismatch totals 0.42 mm summed across the run (FS budget: tens of mm). The connector breakout uses the all-L1 around-the-row scheme from Routing Guide §3.1 — verified, no crossover via. Nearest non-pair signal copper to the pair is 0.43 mm edge-to-edge (one spot, and it's DC VBUS — my informal 0.5 target, real requirement far looser). Solid GND1 0.21 mm under the whole corridor; nothing routed on either inner layer, anywhere (0 segments — checked).

**VBUS chain (LAW 7):** J1 VBUS pads → **D1 at 3.3 mm** (TVS first, stub in single-digit mm ✓) → F1 → C2 → onward as +5V_PROT. D1's GND pad has its via at 1.25 mm. R3/R4 (5.1 k) present on CC1/CC2 with short stubs. C1 sits ~5 mm from U1 pin 5 — on the VBUS cluster rather than at the pin; fine at FS (the clamping is D1's job), noted for completeness.

**Power (LAW 13/14, STRONG 33/39):** every segment on +3V3 / +5V_PROT / VSYS / VBAT / VBUS measures exactly 0.5 mm (Power class enforced, ≈1.4 A capacity vs ≤0.5 A demand). VSYS is an all-top, zero-via compact star. The +3V3 trunk enters C9 (22 µF) → C10 (100 nF) → module pin 2 in that physical order — flow-through past the caps into the pin, exactly as specified; same discipline at the LDO input (C6 at 1.77 mm closer than C5 at 2.78). All power layer-hops doubled except the one VBUS via (§2.2).

**Decoupling (LAW 11/12):** all 16 fitted capacitors have their GND pad's via at **1.12–1.45 mm** — at the pad, every one. Supply-side pad-to-pin: C10 2.0, C14 2.0 (ADC_SOIL at pin 39), C17 2.1 (BAT_SENSE at pin 38), C7 1.9, C8 2.2, C13 2.2, C15 1.9, C16 2.1 mm — the non-negotiables are all at their pins. U1's GND pin (ESD dump) has its via at 1.37 mm.

**Grounds & zones (LAW 19/20/21):** GND1/GND2 fill as single uncut sheets with 0.5 mm edge pullback; B.Cu pour is one region; all four zones now at clearance 0.3 / min width 0.25 / thermal gap 0.3 / spoke 0.5 / remove-islands on. Top pour: main sheet (~1780 mm², 104 GND vias) plus the module island (4 vias), the R11/R14/R15 patch (1 via), the R13 patch (1 via) — the two J1 scraps are §1.2. GND exists on F.Cu only as short via-ties (58 stubs, 70 mm total) — never a long skinny trace. TP7 (scope ground) now has a via at 1.95 mm; TP8 within 2.8 mm.

**Thermal (LAW 15/16/17):** U2's GND pad solid-connected with **5 vias within 3 mm** into two full planes; U6 solid-connected (§2.4); U3 pads 1/40/EP solid. BME280 is 33–45 mm from LDO/charger/module, its labels sit outside the package, no silk under the vent, and all THT parts (J2/J3, TP trio) keep thermal reliefs — hand-solderable, exactly per plan.

**Analog (LAW 18):** ADC_SOIL runs J2 → right side → C14 → pin 39, 21+ mm from the USB corridor, never through the power spine; BAT_SENSE is a 4 mm hop R14/R15 → C17 → pin 38 plus the TP5 spur. Both pass ~2.3–2.8 mm from the (already off-board) antenna keep-out at the pin end — forced by the module's own pinout, microstrip over solid ground, cable exits 38+ mm away; the standing waiver is sound.

**Antenna (LAW 1–4):** module top 6.5 mm including the entire antenna section overhangs the edge; the enlarged keep-out rectangle in the module footprint (x 41.885–89.885, y 28.5–49.5) sits wholly off-board — which is also *why* DRC is silent about it and why the absent 4-layer rule area is genuinely moot for copper (nothing on-board can be "under" the antenna). Pour/plane pullback confirmed 0.5 mm at the top edge on all four layers. Off-board conductor exits (J2, J3, UART TPs) are 36–42 mm away, openings facing away. Only open item is the fence (§1.1).

**Assembly & files (LAW 22–31):** all 51 fitted footprints on top, every one carrying an LCSC number; C3/C4 = dnp + excluded from BOM and CPL; all 12 TPs and H1–H4 excluded from both files; J1 nose overhang measured from the fab outline: **0.96 mm** (HRO target ~1.0); J3 polarity silk verified against the netlist — pin 1 (left) is VBAT_RAW and carries the `+`, pin 2 GND carries the `−`; J2's SIG/PWR/GND read left-to-right over pins 1/2/3 = ADC_SOIL / SOIL_PWR / GND; D1/D4 cathode bars sit on the cathode pads (D1 pad 1 → VBUS, D4 pad 1 → VSYS — correct for both); LED cathode marks on the GND/STAT sides; BOOT label at SW1, RESET at SW2, matching the nets. Silk text uniformly 1.0 mm / 0.15 — at the JLC floor, nowhere below it.

---

## 4. Order-day reminders (unchanged from my own docs, repeated because they're live values)

4 layers · **JLC04161H-7628** · 1.6 mm · impedance control **No** · HASL(LF) or ENIG · **Standard assembly** (module C2913198 is Standard-tier). In the JLC placement preview, check rotation/polarity on D1–D4, U1–U6, Q1–Q3, J1, SW1/SW2 — U1 is placed at 45°, which is legal for assembly but exactly the kind of part whose rotation JLC's importer sometimes mangles, so give it two extra seconds in the preview. Re-check live stock (10 k alternate C98220 noted in my BOM). Then freeze gerbers/BOM/CPL into `fabrication/revA/`, commit, push. Close KiCad and save before exporting — the `.lck` files show it's open right now.

---

*KiCad 10.0.5 headless DRC + independent s-expression geometry audit · companion to `Finishing_Review_RevA_2026-08-16.md` · file under `docs/reviews/`.*
