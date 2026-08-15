# Routing Guide — ESP32-S3 Plant Monitor Rev A, 4-Layer
*Written 2026-08-12 against the saved board file (placement v3: J1 nose overhang now 0.96 mm ✓, TP10 placed at (83, 92) ✓, TP1–TP7/TP9 still parked, SCL/SDA still on pins 4/5, board file still 2-layer — Section 0 converts it). Stack: **L1 signal · L2 GND · L3 GND · L4 signal.** Companion to `Hard_Rules_Layout_RevA.md`; where a LAW number appears, that's the rule being applied.*

---

## 0 · Setup — do these before the first track

**0.1 Convert the board to 4 layers.** Board Setup → Physical Stackup → set copper layers to 4. Rename `In1.Cu` → `GND1` and `In2.Cu` → `GND2` so I never mistake them for routing layers. Enter JLC's standard 4-layer stack (JLC04161H-7628 — verify at jlcpcb.com/impedance): F.Cu **0.035** · prepreg 7628 **0.2104** (εr 4.4) · GND1 **0.0152** · core **1.065** (εr 4.6) · GND2 **0.0152** · prepreg **0.2104** · B.Cu **0.035** — the "board thickness from stackup" readout should land at ≈1.60 mm. That 0.21 mm prepreg is the star of this whole document: every trace on an outer layer now has a ground plane a fifth of a millimetre away.

**0.2 Create the two plane zones immediately.** Add a zone on GND1, net `GND`, covering the whole board; same on GND2. Solid fill, clearance 0.3, min width 0.25 mm, pad connection **thermal reliefs**, remove islands (same numbers as `KiCad_Settings_RevA.md` §9). Fill them now — from the first track I route, the planes are already real, and KiCad will show me if I ever accidentally cut them.

**0.3 Vias.** Keep my 0.6/0.3 default everywhere; 0.45/0.2 is a comfortable smaller size that stays well inside JLC's 4-layer capability if I'm ever squeezed. **Through-hole vias only** — blind/buried vias exist but jump me into a different (expensive) process for zero benefit here.

**0.4 Impedance control: not needed.** The ESP32-S3's native USB is Full Speed (12 Mbit/s) — it tolerates ordinary traces. Order the board as plain 4-layer, "impedance control: no". (If I ever carry this stackup to a High-Speed design, JLC's calculator gives me the 90 Ω pair geometry; at FS my 0.25/0.2 USB class is fine as-is.)

**0.5 Finish the pre-routing edits while edits are still free:**
- Place the remaining eight test points (table in `Placement_Review_RevA_2026-08-12_v2_status.md` — still valid; my TP10 spot at (83, 92) checks out, feed it with a spur from R6).
- Decide the SCL/SDA re-pin (IO47/IO48) **now** — it's a schematic edit + F8, and F8 after routing means ripping up tracks.
- Lock J1 and U3 (H1–H4 already are).

---

## 1 · The one idea that makes a 4-layer board work: return current

Every signal is a **loop**. Current leaves a pin, travels down my trace, and *must come back* — and at any speed above DC it comes back through the ground plane **directly underneath the trace**, hugging its shadow, because that's the lowest-inductance path. My job when routing is really to route the *loop*, not the trace. On this stackup the return path is 0.21 mm below every trace I draw, which is why 4-layer boards feel like cheating after 2-layer: loop areas shrink ~10×, and with them noise, EMI, and crosstalk.

Everything below falls out of that one idea:

1. **Never route on GND1/GND2.** One trace on an inner layer carves a trench through someone else's return path (LAW 19's "no trenches", now applied to two layers). The planes are load-bearing infrastructure, not spare room.
2. **L1 is home; L4 is the escape hatch.** All parts are on top, so a trace that stays on L1 needs zero vias and references GND1 its whole life. Drop to L4 only to cross something I can't untangle — and come back up.
3. **When a signal changes layers, its return current must change planes too.** The signal via carries the signal from L1 down to L4; the return, which was flowing on GND1 under the old segment, now needs to flow on GND2 under the new one — and the only way current moves between two planes is through a ground via. How much this matters scales with signal speed: **for fast edges (the USB pair, if it ever changed layers) a companion GND via within 1–2 mm is important; for slow nets (I²C, EN, IO0, LEDs, SENS_PWR_EN, charger STAT) it's optional** — their return currents happily spread through the next stitching via a few mm away. In practice my ~5 mm stitching grid means most layer changes get a nearby ground via for free anyway; think of the companion via as mandatory for fast signals and free-but-unnecessary insurance for slow ones.
4. **A solid plane can't be crossed wrongly.** There are no splits to straddle because both planes are pure GND — one of the two reasons GND/GND beats the old signal/GND/power/signal stack. The other: my L4 traces get a real reference too, not a patchy power plane.

**What this retires from my 2-layer rulebook:** LAW 6's "nothing on the bottom layer under the USB pair" is now automatic — there are two solid planes between the pair and L4, so routing under it on L4 is harmless. LAW 19/20/21 now read: *planes stay solid, GND is never a trace, stitch the outer pours to the planes.* Everything else in the rulebook stands unchanged.

---

## 2 · Layer plan for this board

| Layer | Job |
|---|---|
| **L1 (F.Cu)** | ~95 % of all routing: the USB pair (entire run), all power, all analog, most signals. GND pour over the leftovers at the end. |
| **L2 (GND1)** | Solid ground. Nothing else, ever. |
| **L3 (GND2)** | Solid ground. Nothing else, ever. |
| **L4 (B.Cu)** | Full routing layer for anything that's cleaner down there — SENS_PWR_EN crossing the board middle, I²C or LED/EN legs where they'd knot on top. Electrically it's first-class (solid GND2 reference at 0.21 mm); prefer L1 only for *workflow* reasons — every part's pads are on top, so L4 costs two vias per visit. A clean L4 route beats a tangled L1 route. GND pour over the leftovers. |

Keep a mental budget: if a net needs more than one trip to L4 and back, rotate a part or rethink the path instead (my guide's old advice stands — untangle with `R`, not heroics).

---

## 3 · Routing order — nets with the least freedom first

**3.1 USB, first and complete.** The pair owns its corridor before anything else claims it.

- *The crossover at J1* (the pads run B7 D− · A6 D+ · A7 D− · B6 D+, interleaved). **Corrected 2026-08-12:** an earlier revision claimed one layer-hop was topologically forced here. That's wrong — it's only forced if both ties *and* both exits stay on the board side of the pad row. Route around the row instead and the whole breakout stays on L1, zero vias (verified against this footprint at my 0.2 mm clearance — all lanes clear by 0.25 mm):
  1. Tie **B7 → A7 (D−)** *behind* the pad row, between the pad toes and the shell — the span (y 76.75–77.75) clears both Ø0.65 peg holes (y 74.61 / 80.39).
  2. Tie **A6 → B6 (D+)** *in front*: from A6's toe, arc right past A7's toe (apex x ≈ 41.8 — 0.4 mm short of U1's courtyard) and into B6's toe.
  3. Exit **D− rightward from B7's lane (y ≈ 76.75)** and **D+ from B6's lane (y ≈ 78.25)**. Each lane clears the neighboring pads by 0.25 mm, and the two lanes land almost exactly on U1's pin 1 (76.55) and pin 3 (78.45) — they flow straight into the ESD chip.
  These first ~2 mm run as two parallel traces ~1.5 mm apart rather than a tight pair — at Full Speed that is completely irrelevant. The tight coupled pair begins at U1's output side. *Fallback only if this fights me in practice:* one ~1.5 mm hop to L4 for the B6→A6 tie, via pair plus a companion GND via — acceptable, but the all-L1 version is cleaner and now known to fit.
- *The run:* A6/A7 → U1 pins 1/3, out pins 6/4 → R1/R2 → module pins 13/14. Route with the differential-pair tool (it will engage now that the nets are named `…DP/…DN`), **all on L1**, side-by-side at 0.25/0.2, matched within a millimetre (that's generous at FS), no further vias, other copper ≥ 0.5 mm away. The corridor from U1 to R1/R2 passes below the LED cluster — route it before the LED lines exist.

**3.2 VBUS in copper order (LAW 7).** J1's VBUS pads → **D1 first** → F1 → onward as +5V_PROT. 0.5–0.8 mm wide, stubs measured in millimetres — the TVS only protects what sits *behind* it in copper, and every millimetre of stub to D1 is ~0.6 nH of clamp delay. Tap C1 off at U1's pin 5, drop C2 right after F1. Then R3/R4 CC drops: short stubs to the CC pads, other ends straight to ground vias.

**3.3 The power spine.** All on L1, ≥ 0.5 mm (the Power class enforces it), wider (0.8 mm) where it's short and easy:

- **+5V_PROT:** from F1 two branches — south-east along the board bottom to C15 → U6 VDD, and north to D4's anode, with the R16/Q3-gate spur off it.
- **VSYS (LAW 13):** the hand-over node — D4 cathode, Q3 source, C18, C5, C6, into U2 VIN. Keep it the fat, compact star it already is in placement; this is the one net where "wide and short" is the entire spec.
- **+3V3:** out of U2 through C7, then **flow past the caps into the pin** (STRONG 33): trunk east along the top to C9 → C10 → module pin 2 — the supply enters the big cap, then the small cap, then the pin, in that order, as one continuous path rather than caps hanging off stubs. Second branch south-east to the sensor corner (R8/R9, C13/U5, C11/C12/U4), with spurs to R5, R7, R6, R10, Q1.
- **VBAT:** J3 pin 1 → Q2 → C16 → U6 VBAT (already a straight line in placement), plus the sense tap north along x ≈ 80 to R14. 

**3.4 Analog (LAW 18).** The quiet pair, routed while the board is still empty enough to give them space:

- **ADC_SOIL:** J2 pin 1 → past my TP6 → up the right side → C14's pad → pin 39. Solid plane under it the whole way (free now), ≥ 0.5 mm from power trunks, nowhere near the USB corridor or the top-edge strip.
- **BAT_SENSE:** R14/R15 junction → C17 → pin 38 — 4 mm total. The divider's VBAT feed routes as ordinary power.

**3.5 Slow signals** — EN (R7/C8 → pin 3, spur to SW2 and TP9), IO0 (R6 → pin 27, spurs to SW1 and my TP10 at (83, 92)), SENS_PWR_EN (pin 23 → Q1's gate — fine to hop to L4 to cross the middle; companion via), SOIL_PWR (Q1 → J2 pin 2), I²C (pins 4/5 or 47/48 → R8/R9 → both sensors; route SCL/SDA loosely as a pair for tidiness — electrically relaxed at 100–400 kHz), LED lines (R5 → D2, R12 → D3 ← U6 STAT — long and slow, route them last through whatever's left), UART (pins 36/37 down the right side and west along the lower board to TP11/TP12 — LAW 34: the path matters, the length doesn't), R13 PROG.

**3.6 GND — the net I don't route.** The pads where a dedicated via *at the pad* is required (LAW 12 plus the heavy hitters): **all ~20 decoupler ground pads** (C1–C18's GND ends — this is the one Espressif calls out explicitly), U1 pin 2 (ESD strikes need the shortest possible path into the planes), the LDO and charger grounds (thermal + return), J1's four shield holes *and* its GND pads (they take the cable yank — solid ties), and TP7/TP8. The module's centre pad already carries its 12. Truly trivial grounds — the 100 k pull-down ends (R11, R15's bottom), R13's ground end, the switch ground pads — don't *need* personal vias; letting them join a well-stitched L1 pour is fine. A via there is still free and harmless, so don't agonize — the only real rule is: never let a trivial pad's only connection be a long skinny pour neck (DRC's unconnected check plus a glance at the filled pour catches this). On this stackup each of those vias is a ~0.25 mm hop to the plane — my decoupling loops land well under 5 nH (pocket-number target: <5 nH = good) almost by default. This is the payoff of the whole 4-layer decision.

**3.7 Pours and stitching.** After routing: GND pour on L1 and L4, then stitch — every decoupler already has its via; add a **perimeter fence** of GND vias around the board edge (~3 mm pitch along the antenna-end edge, ~5 mm elsewhere), vias tying the L1 pour down wherever it forms islands or long peninsulas (LAW 21's ~5 mm grid), and the ring **around** the antenna keep-out — never inside it (LAW 3). With the antenna fully overhanging, the on-board keep-out is automatically satisfied; the fence along that top edge is my RF hygiene.

**3.8 Cleanup.** Refill zones → DRC → fix → repeat until clean; delete dead islands; then the silkscreen pass (LAW 27: +/− at J3, cathode marks, pin-1 marks, BOOT/RESET, TP names by function, nothing on pads or the BME vent).

---

## 4 · Via craft

- **One size fits this board:** 0.6/0.3 for everything. Drop to 0.45/0.2 only where geometry forces me.
- **Fast-signal via ⇒ companion GND via** within 1–2 mm (Section 1). Slow-signal vias can lean on the stitching grid; layer-hop clusters share one companion.
- **Power layer-hops get doubled vias** (STRONG 39) — a 0.3 mm drill is good for over an amp, so this is hygiene, not necessity, at my ≤ 500 mA.
- **No via-in-pad** — a via inside a solder pad drinks the solder down its barrel during reflow and starves the joint. The module's thermal pad is the designed exception (its paste windows account for it).
- **Don't perforate the planes:** a tight row of vias is a perforated tear line — the antipads merge into a slot that blocks return current. Keep ≥ 0.5 mm between barrels and stagger rows. Ten scattered ground vias beat ten in a line.
- **Tent everything** (default): mask over via = no accidental shorts under probe clips. Test *points* are pads, not vias — they stay open by design.
- **Teardrops** (Edit → Fill/Teardrops): optional; mildly nicer for 0.3 mm drills on 0.25 mm traces. Fine to skip.

---

## 5 · Power and heat on four layers

The LDO's pour plan improves for free: L1 copper on its GND pin, 4–6 vias, and now those vias land in **two full-board copper sheets** — a far better heatsink than the 2-layer plan for its ~0.34 W worst case. Same for the charger's ~0.2 W. Keep both pours generous.

The flip side, worth knowing: solid planes conduct heat *everywhere*, including toward the BME280. Distance is still my tool (it's 33–45 mm from all three sources) and the planes also *dilute* hot spots, so the net effect at my power levels is a small, roughly uniform rise — expect the BME280 to read maybe a degree high during long Wi-Fi sessions. Don't slot the Rev A planes for it (plane integrity is worth more); characterize the offset at bring-up with the thermocouple step my review already prescribes, and correct in firmware. Rev B can add thermal slots if the data says so.

---

## 6 · KiCad zone mechanics

Four zones total: GND on L1, GND1, GND2, L4. Same net, so priorities don't matter. Settings that matter:

- Clearance 0.3 / min width 0.25 / **remove islands: below area limit** (kills orphan slivers automatically).
- Pad connection: **thermal reliefs** — critical for J2/J3 and the THT test points, because a pin welded straight into two internal planes is a heat sink my iron can't win against (LAW: hand-solderable stays hand-solderable). SMD decoupler pads work fine through their adjacent via regardless. The module pad's solid connection is set inside the footprint and overrides the zone — leave it.
- **Refill before every DRC** (`B`) — zones don't refill themselves, and DRC on stale fills lies to me.
- Pour L1/L4 *after* routing, not before — routing through existing pour means every edit re-triggers fill churn.

---

## 7 · Interactive routing, practically

Work in **shove mode** (Route → Interactive Router Settings → Shove) — it nudges finished tracks aside instead of blocking me. `X` route, `V` drop a via mid-route, `/` swap posture, `D` drag to tidy, `E` set width mid-route. Highlight-collisions mode is for experts having a bad day; shove is for making progress.

Rhythm that keeps me sane: route one section from §3's list → refill → quick DRC → commit to git. DRC every half hour, not at the end — a clearance error found early is a nudge; found at the end it's an excavation. Lock the USB pair and the VBUS chain once they're perfect so shove can't disturb them. And if the ratsnest looks knotted in some corner, stop routing and stare — the fix is almost always rotating a part (recheck its courtyard gaps after; I have pairs at 0.015 mm that predate me adding breathing room).

Two habits from the pros: color my critical nets (right-click net in the Nets panel → color) so USB/VSYS/analog read at a glance; and when a trace fights me for the third time, delete it and come from the other end — traces routed *from* the fussy pad outward almost always land cleaner.

---

## 8 · Definition of done

Routing is finished when: unconnected items = **0** · DRC = **0** with the real constraint values (no waivers I can't explain in one sentence) · all four zones filled, stitched, no orphan islands · every GND pad has its via · every signal via has a companion · antenna end visually clean with its fence · 3D view pass (connector overhangs, nothing tall by the plug, button reach) · silk pass done · then the pre-order gauntlet from the placement review stands: JLC assembly preview for every polarized part, C3/C4 + TPs + holes excluded from BOM/placement, live stock re-check, **order as 4 layers, stackup JLC04161H-7628, impedance control "no"**, freeze into `fabrication/revA/`, commit, push.

---

## 9 · Advanced corner cases (know these exist; most won't bite)

- **Acute angles & slivers:** never let two traces meet below 90°, and don't leave copper slivers thinner than ~0.2 mm between pours and pads — etchant pools in acute wedges ("acid traps"). 45° habits (STRONG 38) prevent all of it.
- **Annular ring math:** my 0.6/0.3 via leaves (0.6−0.3)/2 = 0.15 mm of ring — exactly the minimum my own DRC enforces (`min_via_annular_width 0.15`). That's why I never shrink the pad while keeping the drill.
- **Mask slivers at the USB pads:** the 0.5 mm-pitch middle pads leave ~0.2 mm mask webs — at JLC's capability, fine; don't add mask expansion there.
- **Plane pullback at the edge:** my 0.5 mm copper-edge rule (LAW 23) applies to the inner planes too — KiCad's zone clearance handles it; just don't override it to zero.
- **Silk discipline:** silkscreen over a via gets drilled into confetti and over a pad gets scraped off — DRC's "silk over pads" check catches it; run it.
- **Fiducials:** JLC's assembly doesn't require board fiducials at prototype scale (they use panel marks). Skip for Rev A.
- **The corner arcs:** copper and courtyards must respect the R4.25 arcs, not the old rectangle corners — the 0.5 mm edge clearance follows the arc. KiCad measures to the actual outline, so trust DRC, not my eyes.
- **Don't chase prettiness into risk:** a routed, DRC-clean, slightly ugly board beats a beautiful one I re-ripped four times. Rev A's job is to work and to teach; Rev B gets to be pretty.

---

*board state measured from disk, not screenshots · LAW/STRONG references are to `Hard_Rules_Layout_RevA.md`, which remains in force except where Section 1 explicitly retires the 2-layer-specific mechanics.*
