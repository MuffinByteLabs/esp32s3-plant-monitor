# Rev A — JLCPCB Order Notes
*Written 2026-08-17, before the first order. Everything on this page is decided; upload day is execution, not decisions. Freeze the ordered gerber zip, BOM, and CPL into this folder afterward, plus the order-confirmation PDF.*

*Updated 2026-08-21: the bottom silkscreen is now populated (board ID block + `JLCJLCJLCJLC` order-number token) — B.Silkscreen is a required plot layer; see §1 and §4.*

## 1. PCB settings

| Setting | Value | Why |
|---|---|---|
| Layers | **4** | L1 signal · GND1 · GND2 · L4 signal |
| Stackup | **JLC04161H-7628** | Matches the numbers entered in Board Setup (0.035 / 0.2104 / 0.0152 / 1.065 / 0.0152 / 0.2104 / 0.035, ≈1.6 mm) |
| Thickness | 1.6 mm | — |
| Impedance control | **No** | USB is Full Speed; pair is at the ~90 Ω geometry anyway (0.29/0.2), ordered as plain 4-layer |
| Surface finish | **HASL (lead-free)** | Fine for 0.5 mm-pitch USB-C; ENIG optional if flatness is wanted — pick explicitly, the KiCad stackup metadata (`copper_finish: None`) does not control the order |
| Solder mask | Green (any) | — |
| Via covering | ~~Tented (default)~~ → **Plugged** (as ordered) | "Tented" is no longer offered on 4-layer — Plugged is JLC's 4L standard at $0 and strictly better; the gerbers (no via mask openings) were already correct for it. Test *points* are pads and stay open by design |
| Remove order number | **"Specify a location"** — the `JLCJLCJLCJLC` token is placed (B.Silkscreen, under J2/J3, added 2026-08-21) | JLC prints the order number at the token and nowhere else |

## 2. Remark field — paste this text

> U3 (ESP32-S3-WROOM-1) antenna overhangs the TOP board edge by ~6.5 mm and the USB-C shell overhangs the LEFT edge by ~1 mm. Please keep panel rails and breakaway tabs clear of the top edge (the overhang must sit over empty space during assembly), and place tabs away from the right edge near the sensor components. Thanks.

Mechanism, for the record: the rail is coplanar with the board, and the module's underside sits only ~0.05–0.1 mm above the board surface — a rail under the overhang can stop the module seating flat in paste. The USB (0.96 mm) and JST (~0.4 mm) overhangs clear a standard ~2 mm routed gap on their own; only the antenna outruns it. V-scoring is impossible on this outline anyway (R4.25 corners force tab-routing), hence the tab-location request. Tabs near the right edge risk depanel flex stress on C11 (3.4 mm from the edge).

## 3. Assembly settings

- **Standard PCBA** (the module, C2913198, is Standard tier). Top side only. J2/J3 (JST PH THT) are through-hole — confirm they're included in assembly, or hand-solder them at bring-up (10 minutes, and the TP trio is THT anyway).
- Confirm in the BOM preview: **C3, C4 absent** (DNP, excluded both sides, parity-verified 2026-08-17) · **all 12 TPs and H1–H4 absent** from BOM and CPL.
- **Rotation/polarity check in the placement preview — every polarized part:** D1 D2 D3 D4 · U1 U2 U3 U4 U5 U6 · Q1 Q2 Q3 · J1 J2 J3 · SW1 SW2. Give **U1 two extra seconds — it is placed at 45°**, the exact case importers rotate wrong. LEDs are the classic victims; the preview render is the truth, not the CPL numbers.
- Live stock check at upload. **10 k = C98220** (C25804 pre-order failed 2026-08-18; BOM updated).

## 4. Fabrication files

Preferred: the kicad-jlcpcb-tools plugin (gerber zip + BOM + CPL with LCSC numbers in one step).

Manual fallback — plot **exactly this layer list** (do not upload the default plot-everything set; the saved plot parameters include User.Comments with the floorplan notes, courtyards, and fab layers):

```
F.Cu  In1.Cu  In2.Cu  B.Cu  F.Mask  B.Mask  F.Paste  F.Silkscreen  B.Silkscreen  Edge.Cuts
```

plus Excellon drill files (PTH and NPTH separate). B.Paste may be omitted (verified empty — no bottom-side parts or paste). **B.Silkscreen is required as of 2026-08-21** — it carries the board ID block (name / Rev A / date / MuffinByteLabs.com / designer) and the `JLCJLCJLCJLC` order-number token; the jlcpcb-tools plugin includes it automatically. Before upload, open the zip in a gerber viewer once: four copper layers, both masks, top paste, **both silks** (bottom shows the mirrored ID block + token), outline with the four corner arcs, plated USB shield slots, PTH + NPTH.

## 5. Pre-upload gate (state on 2026-08-17)

- [ ] Two antenna-fence vias added at ~(56, 51.2) and ~(75.5, 51.0) — last open must-do
- [x] Name/rev + `JLCJLCJLCJLC` texts — **done 2026-08-21** (bottom-silk ID block, order-number token, 2× logo as G1, filled title block)
- [ ] Optional: double the VBUS via at (46.40, 73.95); silk newline cleanup done except one `BME280` comment-box label (docs layer only)
- [ ] Refill zones (`B`) → DRC: **0 errors · 0 unconnected · 0 parity** (last verified clean 2026-08-17)
- [ ] git commit "pre-order baseline", then generate the fab files from that commit
- [x] After ordering: freeze zip/BOM/CPL here — **done 2026-08-21** (copied from `hardware/jlcpcb/production_files/`); **still to do: add the order-confirmation PDF, commit, push**

Then `docs/BringUp_Guide.md` becomes the active document — step 1 (meter the battery plug before it touches J3) stands.

---

## 7. As ordered — 2026-08-21 ✅

**Order placed 2026-08-21.** 5 boards, fab + Standard PCBA, paid ≈ **$49.26 after coupons** (PCB $12.10 + PCBA $86.79 − ~$49.25 coupons) + **$28.67 DHL Express (DDP — duties prepaid, nothing billed later)**.

**PCB as ordered:** 4L · 1.6 mm · green / white silk · FR4 TG135 · **LeadFree HASL** (+$5.10) · Specify Stackup **No** (JLC standard 4L = the 7628 stackup) · **via covering Plugged** ($0; Tented not offered on 4L) · via plating Not Specified · min-via tier 0.3/(0.4/0.45) · outline ±0.2 · flying probe · **Mark on PCB: Remove Mark** (free; JLC strips the `JLCJLCJLCJLC` token in CAM — token kept in the design as insurance).

**Assembly as ordered:** **Standard PCBA** (Economic refused **U3 AND U4 — both Standard-only parts**) · top side · qty 5 · tooling holes/rails **added by JLCPCB** · parts self-service · high-temp paste · nitrogen reflow · **remark submitted** (antenna/rails text) → "quote after review", expected $0 — **answer any JLC engineer email same-day; production pauses until then**.

**Cost anatomy (PCBA $86.79):** setup $25.56 · stencil $8.21 · feeder loading $38.25 (13 Extended lines) · SMT $2.04 · **hand-soldering J2/J3 $3.58 + manual $0.46 (§3's question: yes, the THT JSTs are assembled by JLC)** · X-ray $8.20 (module EP) · packaging $0.49 · **components $0.00 — all 27 unique parts consumed from My Parts Lib**.

**Order-day lessons (read before the next order):**

- **The JLC BOM-matching page is the only live stock truth.** The plugin's offline DB showed C25803 at 9.3 M; the matching page showed **0** and auto-sourced the two 100 k lines from that empty global stock — re-pointed them to My Parts (30 owned) via the row's search dialog. Same failure mode that killed the C25804 pre-order.
- The five 10 k still carried C25804 in the design fields — reassigned to **C98220** in the jlcpcb-tools plugin pre-generation (the plugin writes the LCSC field back to the board).
- Extended lines reserve extra units for **feeder-loading attrition** (15/15 D1, 20/20 D2, 7/7 U1, 6/6 J2…) — expected, not an error.
- Some part rotations were corrected **inside JLC's placement preview** after CPL upload (pin-1 table derived from the board file; U1's 45° imported correctly; D4 stripe → JST side, LED cathodes → USB side, BME280 dot = pin 1 at the silk triangle, second dot = lid vent). **The uploaded CPL is pre-correction — JLC's order preview / downloadable production files are the rotation truth for Rev A.**
- Checkout shipping list: the "My DHL/UPS/FedEx Account" rows are for holders of their own courier accounts ($2.50 = handling only; freight + duties + ~$35–45 brokerage would bill to that account). Global Standard Direct Line caps declared value at $59 — under this order's value. **DDP courier is the correct pick for US orders.**

## 6. Live stock check — LCSC retail, 2026-08-17 (re-verify in the JLC library at upload; the two inventories are separate pools)

Healthy (need ≤ 10 of each, stock in the thousands-to-millions): module C2913198 (4.7 k) · BME280 C92489 (10.9 k) · VEML7700 C1850416 (786) · MCP73831 C424093 (4.2 k) · USBLC6 C7519 (27 k) · AO3401A C15127 · SMF5.0A C2980403 · SS14 C2480 · both LEDs C2289/C2286 · fuse C371166 (6.6 k) · USB-C C165948 · JST C157929/C173752 · switches C318884 · 22R C23345 · 100k C25803 · 470R C23179 · 5.1k C23186 · 100nF C14663 · 4.7µF C19666 · 10µF C15850.

Out of stock at LCSC on check date — expect to resolve at upload:

| Part | Refs | Plan |
|---|---|---|
| AP2112K-3.3 (C51118) | U2 | JLC pre-order (~$9 / 67 min, leftovers bank in My Parts Lib) — or DNP + hand-solder from DigiKey if lead time is bad. See §3. |
| 10 k 0603 1% | R5 R6 R7 R13 R16 | **Resolved 2026-08-18: C25804 pre-order FAILED** (JLC could not source a quotation; $8.61 refunded) despite huge displayed group stock. **The 10 k is now C98220** (Yageo RC0603FR-0710KL, Extended, in stock) — bought into Parts Lib, ~35 pcs. Select **C98220** for R5/R6/R7/R13/R16 at assembly BOM matching; accepts one ~$3 Extended feeder fee. BOM updated. |
| 4.7 k 0603 1% (C23162) | R8 R9 | Any in-stock Basic 4.7 k 0603 ±1% equivalent from the JLC matcher. Same value/size/tolerance = not a design change. |
| 1 µF 0603 (C15849) | C6 C7 C8 C18 | Any in-stock Basic 1 µF 0603 X5R/X7R **≥ 16 V** equivalent (rails are ≤ 5 V). |
| 22 µF 0805 (C45783) | C9 | Any in-stock Basic 22 µF 0805 X5R equivalent, **prefer 16–25 V** (DC-bias derating on the 3V3 bulk cap). |

Basic-tier staples cycle in and out weekly — if the JLC library shows the original C-number in stock at upload, use it and ignore this table.
