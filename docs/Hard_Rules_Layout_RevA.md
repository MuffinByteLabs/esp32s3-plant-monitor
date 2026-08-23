# Layout Hard Rules — ESP32-S3 Plant Monitor Rev A

*The complete graded rulebook for this board's placement and routing. "LAW" items are physics, safety, or fabrication requirements — violating one produces a real defect. "STRONG PRACTICE" items are professional habits — deviate only with a reason. Compiled 2026-08-05 during the pre-layout study sessions; the mechanisms behind every rule are in `docs/Layout_Readiness_and_Placement_Guide_RevA.md`.*

---

## LAW — physics, safety, fabrication

### Radio
1. Antenna keep-out is absolute on **all layers**: no copper, traces, silk, pour, or parts. Overhang off the board edge preferred; keep-out over bare board is legal but donates range; copper under it kills it.
2. No metal in front of the antenna — hardware, standoffs, screws, case features. ≥15 mm to enclosure walls at case time.
3. Ground stitching vias **around** the keep-out, never inside it.
4. Off-board conductors — battery leads, soil cable, TP wires **and the traces feeding them** — stay ≥15 mm from the antenna zone. The wire's natural resting drape counts, not just the connector position. Exits face away from the antenna. (Mechanism: conductors ≥ ~λ/10 ≈ 12 mm couple to the 2.4 GHz field; cables are the biggest antennas in the system.)

### USB
5. D+/D− routed as a glued pair: side-by-side, constant ~0.2 mm gap, matched length, **zero vias**, top layer only.
6. Unbroken ground beneath the pair and its ±5 mm shadow strip. Nothing crosses beneath it on the bottom layer.
7. Copper meets things **in physical order, and stubs count in millimeters**: J1 VBUS pads → D1 (TVS) within a few mm → F1 → everything else. Data: J1 → U1 flow-through → corridor → R1/R2 at the module end → module pins. (Same net ≠ same point: an ESD wavefront visits things in copper order; every mm of stub to D1 is ~0.6 nH of delay on the clamp.)
8. R3/R4 (5.1 k) must exist on CC1/CC2 — required for USB-C 5 V. Position is soft (DC doesn't care); keep them in the USB cluster.
9. C3/C4 stay DNP. If ever fitted: ≤47 pF, never 100 nF.
10. J1's shell nose overhangs the board edge ~1 mm, exactly per the HRO drawing (edge ~1.6 mm ahead of rear shield-slot centers). Flush/recessed = plug never seats.

### Power integrity
11. Every decoupler at its pin, **millimeters not centimeters**: C9/C10 @ module 3V3 · C5/C6 @ LDO in · C7 @ LDO out · C15 @ charger VDD · C16 @ charger VBAT · C11/C12 @ BME280 VDD/VDDIO · C13 @ VEML VDD · C1 @ U1 VBUS · R7/C8 @ EN · R6 @ IO0. (Mechanism: trace = 6 nH/cm; V = L·ΔI/Δt; the cap is the local tank for demands faster than the LDO's µs loop. Decoupling loop target < ~5 nH total.)
12. Every decoupler's GND pad gets its own via to the plane, at the pad.
13. VSYS node (D4 cathode + Q3 source + LDO input) compact and fat — the USB↔battery hand-over lives here.
14. Power nets ≥0.5 mm (Power net class enforces).

### Thermal
15. LDO (U2): copper pour on its GND pin, stitched to the bottom plane — its only heatsink for ~0.34 W.
16. Charger (U6): modest copper, ~0.2 W into a low battery.
17. BME280 far from LDO, charger, and module; vent hole open to air; no silkscreen on or over it. Board heat = corrupted readings.

### Analog
18. ADC_SOIL and BAT_SENSE: short runs, never alongside the USB pair, near the antenna, or through the power zone. **C14 and C17 (100 nF) at the module's ADC pins** — that is the part that is not negotiable. R11 (100 k pull-down; holds the pin at a defined ~0 V when the probe is unpowered or unplugged) and R14/R15 (the ÷2 divider) are DC jobs — electrically position-free, kept beside their caps for tidiness and one shared ground via. (Mechanism: the ESP32's SAR converter does not "look at" the voltage — it samples by charging a small internal capacitor, ~10 pF, from the pin, so reading a voltage really does draw a brief gulp of charge. A 100 nF cap sitting at the pin is ~10 000× larger, so the sample costs it ~0.01 % of its voltage — ≈0.15 mV, under a fifth of one LSB at 12 bits over 2900 mV — and the source refills it at leisure between readings. Sited anywhere else, that gulp has to arrive through the source's own impedance instead, and the pin sags while the ADC is looking at it.)

*Naming note — corrected 2026-08-08. Earlier drafts called R11/C14 and R14/R15/C17 "filter parts" as pairs. That is only half true, and the halves differ:*

- ***BAT_SENSE genuinely is an RC low-pass.** The divider's Thévenin impedance (R14‖R15 = 50 k) working against C17 gives a ~32 Hz corner. That same 50 k is exactly why C17 must be at the pin: a 50 k source cannot fill the ADC's ~10 pF sampling cap fast enough on its own (τ ≈ 500 ns, ~3.5 µs to settle).*
- ***ADC_SOIL is not a filter as drawn.** R11 sits in **parallel** with the signal, not in series, so there is no on-board series element — R11 is purely a pull-down. C14 is the ADC input cap; the low-pass it forms works against whatever impedance the probe and its cable present, not against anything on this board. Adding a small series R here is a Rev B item.*

### Ground
19. Bottom layer = one continuous plane. No trenches, especially under USB and analog shadows (a slot resurrects the inductance the plane exists to kill).
20. GND is never a long skinny trace anywhere — plane + vias. Module center pad ties into the plane through its via array; keep the pour solid beneath the module.
21. Stitch top pour to bottom plane every ~5 mm and at every decoupler.

### Assembly & fabrication
22. Every part on the top side (economic single-sided assembly — never press F).
23. Copper ≥0.5 mm from board edge.
24. Courtyards clear of each other; DRC clean; unconnected items = 0.
25. Connector openings (J1/J2/J3) face off-board.
26. M3 screw heads: ~Ø6 mm part-free at each hole; no hole or metal standoff in the antenna region.
27. Safety silk: big **+ / −** at J3 (the board's one real hazard is a reversed battery) · cathode marks D1/D4 · LED polarity · pin-1 marks J2/J3 · BOOT/RESET labels · TP names · silk never on pads or the BME vent.

### Process, before ordering
28. ERC and DRC clean, with board↔schematic parity check.
29. JLC assembly preview inspected for rotation/polarity on every polarized part (LEDs are the classic victim).
30. DNP parts excluded from BOM and placement files.
31. Stock and Basic/Extended status re-verified at upload (live values).
32. Fab files frozen into `fabrication/revA/`; git committed and pushed.

---

## STRONG PRACTICE — do unless there's a reason

33. Smallest cap closest to the pin (C10 before C9, C6 before C5); supply trace flows past the caps into the pin, not caps on stubs. (Caps are speed classes: the sprinter gets the short path.)
34. UART (TXD0/RXD0 → TP11/TP12) routed through the lower board, ground pour hugging it. Length is free at UART speeds; only the path matters.
35. R3/R4 in the USB cluster with short CC stubs.
36. R16 beside Q3's gate.
37. No long trace runs parallel along the keep-out border; a couple mm of courtesy space when free.
38. 45° corners (habit, not physics at these speeds); parts at 0°/90°; passives reading consistently.
39. Doubled vias where real current changes layers.
40. Edge-committed parts (J1, U3) placed first and locked.

---

## 4-layer amendments — added 2026-08-13

*The board went from 2 to 4 layers: **L1 signal · GND1 · GND2 · L4 signal** (JLC04161H-7628; both middle layers solid ground — the 08-06 idea of power islands on In2 is superseded). Every rule above stands except as amended here; mechanisms in `docs/Routing_Guide_RevA_4Layer.md`.*

- **LAW 19/20 now cover two planes:** GND1 and GND2 are never routed on and never cut. "Bottom layer = one continuous plane" becomes "both inner layers = continuous planes"; B.Cu (L4) is a normal routing layer with its own GND pour.
- **LAW 6 is automatically satisfied:** two solid planes sit between the USB pair and L4, so routing on L4 under the pair is harmless. The pair itself still stays on L1.
- **LAW 5 stands, and improves:** the J1 duplicate-pad breakout routes entirely on L1 (one tie behind the pad row, one in front, exits from the outer pads — Routing Guide §3.1, geometry verified 08-12), so the pair truly gets zero vias, no exceptions.
- **LAW 12 unchanged for decouplers** — every decoupler GND pad gets its via at the pad (the hop to GND1 is now 0.21 mm — the whole point of the stackup). Amendment: *trivial* grounds (100 k pull-down ends, switch GND pads, R13's ground) may join a well-stitched pour instead of getting personal vias.
- **LAW 21 restated:** stitch the outer pours into the planes every ~5 mm and at every decoupler. New companion rule: a **fast** signal changing layers needs a GND via within 1–2 mm (return current must jump planes through it); slow signals can lean on the stitching grid.
- **LAW 1–3 unchanged**, but the keep-out must now be enforced on **four** copper layers — use a Rule Area covering F.Cu/GND1/GND2/B.Cu (re-create it: the current board file has none).

## Pocket numbers

- Trace: **~6 nH/cm**, ~10 mΩ/cm at 0.5 mm width (1 oz; 0.5 mΩ/square × 20 squares/cm — corrected 2026-08-17, earlier drafts said 1 mΩ/cm) · via ~0.5–1 nH · cap internal ESL ~0.5–1 nH · plane return ~0.1 nH/cm · loose wire ~10 nH/cm · battery leads ~200 nH
- **V = L·(ΔI/Δt)** — shortcut: nH × (A ÷ ns) = V, directly
- ΔV = I·Δt/C (tank sag) · τ = R·C · V = I·R · edge length = 15 cm/ns × edge time (worry when trace > edge/10)
- Fast-path inductance: >20 nH = problem, 5–10 nH = noticeable, <5 nH = good decoupling loop. Slow paths: hundreds of nH are free.
- 2.4 GHz: λ ≈ 12.5 cm; conductors ≥ ~12 mm start coupling to the antenna.
