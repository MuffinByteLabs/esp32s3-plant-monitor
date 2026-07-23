# Independent Pre-Layout Design Review — ESP32-S3 Plant Monitor, Rev A

**Reviewer:** Claude (independent netlist re-extraction + datasheet cross-check)
**Date:** 2026-07-22
**Board:** RS_ESP32S3_PlantMonitor_RevA · KiCad 10.0.2 · 2-layer
**Files reviewed:** all 8 schematic sheets, the project file, the custom symbol/footprint library, the BOM, and the full design document + prior review.

---

## Bottom line

**This schematic is ready for PCB layout.** I did not find a single blocking or functional error. Every net I traced matches what the design intends, every part has a footprint, the power path works, and the design follows Espressif's official ESP32-S3 guidelines. You can confidently run **"Update PCB from Schematic" (F8)** and start placing parts.

The items below are *refinements, layout-time cautions, and things to double-check on the bench* — not reasons to stop. I've sorted them by how much they matter so you can see at a glance that nothing is urgent.

Think of the verdict as: **green light, with a short pre-flight checklist.**

---

## How I checked this (so you can trust the verdict)

Rather than just read the schematic pictures, I wrote a program that re-parsed the raw KiCad files and rebuilt the **netlist** from scratch — the list of which component pins are electrically joined into each "wire." A netlist is the real truth of a schematic; the pretty drawing can hide mistakes that the netlist exposes. I then:

- **Traced the whole power path by hand** — USB in → fuse → 5 V → charger, Schottky diode, and the two MOSFETs that hand power between USB and battery — and confirmed every diode and transistor is turned the right way around. (This is the part most likely to have a silent error, so I verified the orientation of all 4 diodes/LEDs and all 3 MOSFETs individually against their datasheets.)
- **Confirmed the ESP32-S3 pin map** matches your `PinMap_CheatSheet.md` exactly.
- **Checked every component has a footprint** (69/69) and that the transistor's schematic symbol matches the real AO3401A pin order (Gate-Source-Drain), so the MOSFETs will be correct on the copper, not just on paper.
- **Cross-checked the design against Espressif's ESP32-S3 Hardware Design Guidelines** for USB, reset, and boot pins.
- Read your design doc, BOM, engineering notes, and the 2026-07-20 review, and **verified that the fixes from that review are actually in the current files.**

---

## What's correct (the reassuring part)

Everything structural is right. Specifically, I independently confirmed:

**Power & protection**
- USB-C sink wiring is textbook: both CC pins get their own 5.1 kΩ resistor to ground (R3, R4 — one per pin, never shared), all four VBUS pads tied, and — importantly — **all four USB-C ground pins (A1/B1/A12/B12) plus the shield connect to ground.** (This was the big fix from the last review; it's confirmed present.)
- TVS (D1) and ESD array (U1) are oriented correctly and sit at the connector; the polyfuse (F1) carries the full board current with margin.
- The 3.3 V regulator (U2) has its enable tied to its input (so it's always on — a floating enable would be a dead board), and the correct input/output capacitors.
- **The automatic USB↔battery power path is wired correctly** — the Schottky D4 points the right way (5 V → system), the source-select MOSFET Q3 has source→VSYS, drain→VBAT, gate→+5 V with the 10 kΩ pull-down to ground, and the reverse-polarity MOSFET Q2 is oriented to block a backwards battery.
- The battery charger (U6, MCP73831) has the right program resistor for ≈100 mA and correct input/output caps.

**Brain & sensors**
- ESP32-S3 reset network (10 kΩ + 1 µF + RESET button) and boot strap (GPIO0 with 10 kΩ pull-up + BOOT button, **no capacitor** — exactly as Espressif requires) are both correct.
- Native USB goes to GPIO19/20 through the ESD chip and 22 Ω series resistors, with unpopulated tuning caps — this matches Espressif's recommended USB circuit precisely.
- I²C has **exactly one pull-up pair** (R8/R9, 4.7 kΩ) for the whole bus — the most common beginner mistake is adding extra pull-ups, and you correctly avoided it.
- BME280 address is locked to 0x76 (SDO→GND) and forced into I²C mode (CSB→3.3 V); VEML7700 is at its fixed 0x10. No address conflict.
- Both analog inputs (soil, battery-sense) have their 100 nF filter caps at the pin, and unused pins are properly marked no-connect.

**Housekeeping**
- All 69 components have footprints. 12 test points and 4 mounting holes are present. Power-flag symbols are placed on exactly the three rails that need them for the electrical-rules check to pass.

---

## Findings, by priority

| # | Priority | Item | Type |
|---|----------|------|------|
| — | **Blocker** | *(none found)* | — |
| 1 | Watch at bring-up | USB→battery hand-over can dip close to brown-out **at low battery** | Verify on bench |
| 2 | Keep in mind | Reversed battery **while USB is plugged in** can trickle backwards | Accepted risk / procedure |
| 3 | Layout-critical | LDO needs a copper "heat-spreader" pour | Do during layout |
| 4 | Layout-critical | Double-check the 4-pad tactile-switch footprint mapping | Verify during layout |
| 5 | Optional (Rev B) | No ESD protection on the soil/battery connectors | Nice-to-have |
| 6 | Nitpick | Custom VEML7700 symbol uses generic pin types | Cosmetic/ERC hygiene |
| 7 | Process | Push the git commit; re-verify JLC stock at order time | Not a schematic issue |

Details below, in plain language.

### 1 — Watch the USB-unplug hand-over at low battery *(bench check, not a fix)*

When you pull the USB cable, the board has to hand power from the 5 V rail over to the battery. For a few tens of milliseconds during that switch, the system rail is fed through a "body diode" inside Q3, which drops it to roughly **(battery voltage − 0.65 V)**. Do the arithmetic:

- Full battery (4.2 V) → rail ≈ 3.55 V during the switch → fine.
- Nearly empty (3.6 V) → rail ≈ 2.95 V → this is right at the ESP32-S3's default brown-out reset threshold (~3.0 V).

So the *only* scenario with any risk is "unplug USB at the exact moment the battery is nearly flat." The previous review already improved this a lot by changing R16 from 100 kΩ to 10 kΩ (I confirmed it's 10 kΩ), which shortens the switch to ~50–100 ms so the bulk capacitors can ride through it. This is genuinely fine on paper — but hand-over timing depends on real capacitor and transistor behavior, so **put a scope on the VSYS test point (TP2) and unplug USB at a low battery voltage during bring-up** (your bring-up guide already lists this as step 7 — just pay special attention to the low-battery case). If you ever see a reset, the firmware can briefly relax the brown-out detector during the transition.

### 2 — Reversed battery + USB = slow reverse trickle *(known, accepted)*

Your reverse-polarity MOSFET (Q2) protects the board beautifully when running on battery. But there's a subtle corner the last review already flagged and accepted: if someone plugs the battery in **backwards** *and* USB is also connected, the charger can push a small current that turns Q2 on into the reversed cell. It's a slow trickle (not a bang), the board itself is unharmed, and the charge LED will misleadingly look "normal."

There's no cheap fix in this exact topology, so the mitigation is procedural and it's the right call for Rev A: **always meter the battery plug's polarity before the first connection**, and remember that *the charge LED lighting does NOT confirm correct polarity.* Your bring-up guide already says this. If you want it gone in Rev B, that's what an integrated power-path/charger IC (e.g. MCP73871, BQ24075) or a dedicated protection FET solves. I'd keep it as-is for this spin.

### 3 — Give the regulator a copper heat-spreader *(do at layout)*

The 3.3 V regulator (U2) is a small SOT-23-5 part. In the worst case (USB power + Wi-Fi running hard) it burns off about **0.34 W as heat**. That's fine *if* it can shed heat into the board. During layout, pour a generous copper area connected to its ground pin/tab on both layers with a few stitching vias. Your design doc already plans this — I'm just confirming it's a real requirement, not optional, and worth a thermocouple check at bring-up during a long Wi-Fi session.

### 4 — Re-verify the tactile-switch footprint *(do at layout)*

Your two buttons (SW1 = BOOT, SW2 = RESET) use a **custom 4-pad footprint** from your project library. Physical tactile switches have 4 pins that are internally two pairs; the footprint pads were renumbered to 1/1/2/2 to match the 2-pin schematic symbol. The netlist is correct (SW1 connects GPIO0↔ground, SW2 connects EN↔ground), but **4-pad switch footprints are a classic source of "it's mirrored/rotated wrong" mistakes.** Your BOM says this was verified against the datasheet's A-B / C-D pairing — just give it one more look in the footprint editor and on the placed part, since it's cheap insurance and hard to fix after fab.

*(Side note: the schematic, BOM, and pin-map all agree SW1 = BOOT and SW2 = RESET. The main design document's body text has these two swapped — the schematic is the correct source of truth, so just make sure the silkscreen labels follow the schematic, which they will automatically.)*

### 5 — Consider ESD protection on the external connectors *(optional, Rev B)*

The soil probe (J2) and battery (J3) plug in with external wires that a person will touch. Right now the soil signal line has only its filter cap and pull-down; there's no ESD clamp on J2 or J3. For a device that sits by a plant and gets handled, a small TVS/ESD diode on the ADC_SOIL line (and optionally the battery line) would make it more rugged. It's genuinely optional — plenty of hobby boards ship without it — and it's already on your Rev B list, so I'd leave Rev A alone unless you have board space to spare.

### 6 — Custom VEML7700 symbol pin types *(cosmetic)*

In your custom `PlantMonitor_JLC` library, the VEML7700 symbol's power pins (VDD, GND) are typed as generic "input/passive" rather than "power." Everything connects correctly, so this changes nothing electrically — but it means KiCad's electrical-rules-check won't actively watch those pins as power pins. Purely a library-hygiene nitpick; fix it whenever you next touch that symbol, or ignore it.

### 7 — Process items (not schematic problems)

From your own notes, these are still open and worth closing before you order:

- **Push the git commit.** Your `PROJECT_STATUS.md` notes the last milestone commit was made in an environment with no network, so `git push` still needs to run from a normal terminal. Your work isn't backed up until you do.
- **Re-verify JLCPCB stock and Basic/Extended status at upload.** Stock is a live value; the BOM even notes the 10 kΩ (C25804) hit zero stock once and lists an alternate. Re-check every line at order time.
- **The usual build-time rituals** you've already documented: buzz out the soil-probe cable wire order before soldering, and meter the battery plug polarity before first connection.

---

## Layout-phase checklist (your next stage)

Since the schematic is done, here's what actually matters when you place and route — most of it is already in your design doc's Section 9; this is the short version:

1. **Protection parts first, right at the USB-C connector:** fuse F1, TVS D1, ESD chip U1. They only work if the surge hits them *before* reaching anything else.
2. **USB data pair (D+/D-):** keep the two traces short, side-by-side, and over a solid unbroken ground area with no vias between them. A 2-layer board can't hit the textbook USB impedance — "short and clean" is the accepted compensation, and full-speed USB tolerates it well.
3. **Regulator U2:** the copper heat pour from finding #3.
4. **Antenna:** the module's antenna end must overhang the board edge (or have all copper cut away under and beside it), kept ≥15 mm from any enclosure wall, with no copper/traces/silkscreen underneath on either layer.
5. **Sensor placement:** BME280 away from the regulator and the module (so it reads air temperature, not board heat) with its vent hole unobstructed; VEML7700 with a clear view up and away from the two LEDs.
6. **Analog traces (ADC_SOIL, BAT_SENSE):** keep them short and away from the USB pair and the antenna.
7. **Trace widths:** power nets (VBUS, +5V_PROT, VSYS, VBAT, +3V3) ≥0.5 mm; signals ~0.2–0.25 mm.
8. **Run DRC** (design-rule check) with JLCPCB's 2-layer capabilities set, and re-run **ERC** one final time after any last schematic tweak.

---

## Accepted trade-offs (fine for Rev A — just know they're there)

These are deliberate, documented choices, not mistakes:

- **On battery, the 3.3 V rail follows the battery down once it drops below ~3.6 V.** Firmware stops Wi-Fi transmit at 3.5 V and sleeps at 3.0 V to manage it. A buck-boost regulator would fix it in Rev B if you ever need the full battery range.
- **The power LED is the biggest sleep-mode drain** (~120 µA). It's deliberately dim; leave it unpopulated on any board you use to measure true sleep current.
- **You must use a *protected* lithium cell** — the board has no on-board battery-protection chip by design. This is stated as a hard requirement, which covers it.
- **Charge LED may flicker faintly with no battery connected** — cosmetic, harmless.

---

## Summary

| Question | Answer |
|---|---|
| Any electrical errors? | **No** |
| Any missing footprints? | **No** (69/69 assigned) |
| Power path correct? | **Yes** — traced and verified pin-by-pin |
| Matches Espressif guidelines? | **Yes** (USB, reset, boot pins all conform) |
| Prior review's fixes present? | **Yes** — all confirmed in the current files |
| **Ready for PCB layout?** | **Yes — go.** |

This is a genuinely well-executed first board, and unusually well-documented for a portfolio project. Nice work. Run F8 and start placing parts — and keep the bring-up guide handy, because the two things worth watching (the low-battery hand-over and the reverse-battery procedure) are behaviors to confirm with a meter, not schematic changes.

---

*Sources: Espressif ESP32-S3 Hardware Design Guidelines & Schematic Checklist (docs.espressif.com). Findings verified against the project's own datasheets (AO3401A, MCP73831, AP2112K, USBLC6-2SC6, BME280, VEML7700) and the as-built KiCad netlist.*
