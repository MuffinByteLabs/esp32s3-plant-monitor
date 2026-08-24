# ESP32-S3 Plant Monitor — Rev A Design Document

**Board:** 62.5 × 44.5 mm · 4-layer (signal / GND / GND / signal, JLC04161H-7628) · 1.6 mm · lead-free HASL

**Status:** ordered at JLCPCB 2026-08-21 · **Written** 2026-07-04 · **Revised to as-built** 2026-08-23

> **Every reference designator below is as-built.** They match the KiCad schematic, the
> [schematic PDF](ESP32S3_PlantMonitor_RevA_Schematic.pdf) and the
> [ordered BOM](../fabrication/revA/BOM-ESP32S3_PlantMonitor.csv). Earlier drafts of this document
> used planning-stage designators; the full renumbering record is in **Addendum A**.

*This document is the single complete description of the board: what it does, how the circuitry works, every component and why it is there, and how the design will be captured, manufactured, assembled, and brought to life. It is organized to match the KiCad hierarchical schematic sheets, so each section of this document becomes one sheet of the schematic.*

*Plain-language rule used throughout: every abbreviation is expanded where it appears, and repeated later, so no section requires remembering a definition from another section.*

## 1. Project Overview

Goal: a small, manufacturable Wi-Fi plant monitor. It sits by a plant, measures soil moisture, air temperature, humidity, barometric pressure, and ambient light, and reports the readings over Wi-Fi. It is powered by USB-C, and it also runs from a rechargeable single-cell lithium-polymer battery when USB is unplugged. The same USB-C port charges the battery.

The board is a portfolio project: every choice below follows industry-standard practice for a first-spin four-layer board, and every deviation from standard practice is called out openly in Section 15.

### What the board does

- Reads soil moisture with an external capacitive probe (DFRobot SEN0193) plugged into a connector.

- Reads air temperature, relative humidity, and barometric pressure with one on-board sensor (BME280).

- Reads ambient light with a second on-board sensor (VEML7700).

- Connects to Wi-Fi and is programmed over its USB-C port — no external programmer needed.

- Charges a 1-cell lithium-polymer battery and automatically runs from it when USB power is removed.

- Measures its own battery voltage so the firmware can report battery percentage.

- Switches power to the soil probe on and off in firmware, so the probe does not drain the battery between readings.

### Success criteria

- Board powers up on USB-C, enumerates as a USB device, and accepts firmware.

- All sensors read correctly; soil readings are calibrated (dry value and wet value recorded).

- Unplugging USB while running does not reset the board — it hands over to the battery seamlessly.

- Battery charges at roughly 100 mA and the charge LED behaves as described in Section 8.

- Deep sleep current is low enough for multi-week battery operation (estimate in Section 2).

## 2. System Architecture — Sheet 01_System

The top schematic sheet (01_System) holds no components. It contains the hierarchy: one block per sub-sheet, connected by named nets. A net is simply a named wire; two points with the same net name are electrically the same wire, even across sheets.

### Power flow, in one paragraph

USB-C delivers 5 V. That 5 V passes a surge-protection diode and a resettable fuse and becomes the protected 5 V rail, called +5V_PROT. From +5V_PROT, two things happen: the battery charger charges the battery, and a Schottky diode feeds the system supply node, called VSYS. When USB is present, VSYS is about 4.6 V and the battery is disconnected from the load. When USB is removed, an automatic MOSFET switch connects the battery to VSYS instead — with no gap the processor can notice. VSYS feeds a 3.3 V regulator, and the regulator's output rail, +3V3, powers everything that thinks or senses: the ESP32-S3 module, both I2C sensors, and (through its own switch) the soil probe.

### The two working voltages

- 5 V side (+5V_PROT): the battery charger, the charge LED, and the input of the power path. Everything upstream of the regulator.

- 3.3 V side (+3V3): the ESP32-S3 brain, the BME280, the VEML7700, the soil probe, all pull-up resistors, and the power LED. Everything that runs code or measures.

- GND (ground, 0 V): the single shared reference every voltage is measured against. All grounds on the board are one net.

### Net name glossary

| Net name | Meaning |
|---|---|
| USB_VBUS          | Raw 5 V at the USB-C connector, before the fuse.                                                                       |
| +5V_PROT          | Protected 5 V, after the fuse. The 5 V rail everything actually uses.                                                  |
| VSYS              | System supply node: 5 V (minus a diode drop) when USB is present, battery voltage when it is not. Feeds the regulator. |
| +3V3              | The regulated 3.3 V rail from the AP2112 regulator.                                                                    |
| VBAT_RAW          | Battery connector positive pin, before reverse-polarity protection.                                                    |
| VBAT              | Protected battery rail, after the reverse-protection MOSFET. The charger and power path connect here.                  |
| BAT_SENSE         | Divided-down battery voltage (half of VBAT) going to an analog input for battery-percentage measurement.               |
| USB_DP / USB_DN   | USB data plus / data minus, from the connector to the ESP32-S3's native USB pins.                                      |
| SCL / SDA         | The shared two-wire sensor bus: clock line and data line.                                                              |
| SOIL_PWR          | Switched 3.3 V that powers the soil probe only when firmware enables it.                                               |
| SENS_PWR_EN       | The control signal (from GPIO21) that turns the soil-probe power switch on and off.                                    |
| ADC_SOIL          | Soil-probe output going to an analog input — direct, no divider (see §7); R11 (100 kΩ) holds the pin down while the probe is off.                                                               |

### Current budget (worst case, everything peaking at once)

A current budget is a list of every load and its worst-case draw, added up, to prove the supply and the fuse can cover it. Values verified against each part's datasheet.

| Load on +3V3 | Peak current | Notes |
|---|---|---|
| ESP32-S3 module (Wi-Fi transmit peak) | 355 mA           | Budgeted as 500 mA supply capability per Espressif. |
| Soil probe SEN0193                    | ≈5 mA            | Only while switched on by firmware.                 |
| Power LED D2                          | ≈0.12 mA         | On whenever +3V3 is up.                             |
| I2C pull-ups (both lines held low)    | ≈1.4 mA          | Brief, during bus traffic.                          |
| BME280 (measuring)                    | ≈1 mA            | Micro-amps between measurements.                    |
| VEML7700                              | ≈0.05 mA         | Tiny.                                               |
| Total 3.3 V side                      | ≈365 mA          | Regulator capability: 600 mA — passes.              |

On the 5 V side, the fuse additionally carries the battery charge current (about 100 mA) at the same time. Worst simultaneous case is therefore roughly 465 mA. The fuse holds 750 mA at room temperature and 650 mA at 40 °C — comfortable margin either way.

### Power modes

- USB plugged, battery present: board runs from USB; battery charges; charge LED lights until full.

- USB plugged, no battery: board runs from USB normally; charger sits idle (its status pin floats — the charge LED may flicker faintly; this is expected and harmless).

- USB unplugged, battery present: board runs from the battery. The 3.3 V rail stays clean while the battery is above roughly 3.6 V, then gracefully follows the battery downward. The firmware reads BAT_SENSE and reports the level.

### Estimated battery life (deep sleep)

Baseline sleep current: regulator quiescent ≈55 µA, battery-sense divider ≈21 µA, power LED ≈120 µA (dominant — remove D2 for battery-life tests), ESP32-S3 deep sleep ≈10–20 µA — call it ≈210 µA floor as built (≈90 µA with the LED off). The bench cell is a 500 mAh PKCell (halve any 1000 mAh estimates): ≈100 days of pure sleep, and firmware must stop discharge at 3.0 V because the pack’s own protection lets it fall to 2.5 V. This is an estimate to verify at bring-up with a current meter, not a datasheet guarantee.

## 3. USB-C Input & Protection — Sheet 02_USB_C_Input

This sheet brings 5 V power and USB data onto the board safely. Every part here exists to protect something or to satisfy the USB-C specification.

### J1 — USB-C receptacle (TYPE-C-31-M-12, JLCPCB part C165948)

A 16-pin USB-C connector that carries power and USB 2.0 data only (no high-speed lanes — this board does not need them). The connector has two of every contact so the plug works in either flip orientation:

- VBUS pads (A4, B4, A9, B9) are all tied together — this is the incoming 5 V, net USB_VBUS.

- Data pads are tied in pairs at the connector: A6+B6 form D+ (data plus), A7+B7 form D− (data minus).

- CC1 and CC2 (Configuration Channel pins) each get their own 5.1 kΩ resistor to ground (R4, R3). These two resistors are how the board announces 'I am a power-consuming device, please give me 5 V.' One resistor per pin, never shared — sharing breaks orientation detection. Source: ST application note TA0357.

- SBU pins (sideband use): not connected — unused in USB 2.0 designs.

- GND pads (A1, B1, A12, B12): all tied directly to board ground — these carry the return current for VBUS and reference the data lines (fixed 2026-07-20; an early capture left them floating). Shield: connected directly to ground for this revision (standard practice for a small self-contained device).

### D1 — TVS diode on the power input (SMF5.0A, Goodwork, C2980403)

TVS = Transient Voltage Suppressor: a diode that does nothing at normal voltage but clamps hard when a fast voltage spike (static discharge, cable surge) arrives. D1 sits right at the connector across USB_VBUS to ground. Ratings: stands off 5 V, starts conducting at ≈6.4 V, clamps at ≈9.2 V. This satisfies the Espressif hardware-design-guideline recommendation for surge protection at the main power entrance.

### F1 — resettable fuse (PPTC, Littelfuse 1206L075/16WR, C371166)

PPTC = Polymeric Positive Temperature Coefficient device, commonly 'polyfuse.' It is a resettable fuse: too much current heats it, its resistance jumps, current stops; when it cools, it resets itself. F1 sits between USB_VBUS and +5V_PROT, so every milliamp the board uses passes through it.

- Hold current (passes forever without tripping): 0.75 A at 20 °C, derating to 0.65 A at 40 °C.

- Trip current (definitely cuts off): 1.5 A.

- Sized so the worst simultaneous load (≈465 mA: Wi-Fi peak plus battery charging) fits under the hold current even warm.

### U1 — USB data-line ESD protection (USBLC6-2SC6, ST, C7519)

ESD = Electrostatic Discharge, the static zap from a hand or a cable end. U1 is a purpose-built protection array for high-speed data lines: extremely low capacitance so it does not distort USB signals, but it clamps zaps on D+ and D− before they reach the ESP32-S3. It is a flow-through part: connector-side data enters pins 1 and 3, protected data exits pins 6 and 4, so the layout naturally passes the signals through the chip. Its VBUS pin (pin 5) connects to USB_VBUS with a local 100 nF capacitor (C1).

### R1, R2 — 22 Ω series resistors, and C3, C4 — reserved capacitors (DNP)

The Espressif design guidelines ask for series resistors on the USB data lines close to the chip (starting value 22–33 Ω) and reserved capacitor footprints to ground that are left unpopulated. DNP = Do Not Populate: the footprint is on the board, but no part is soldered at the factory. These give tuning options if USB signal quality ever needs adjustment, at zero cost if it does not.

Nets leaving this sheet: +5V_PROT (to power sheets), USB_DP and USB_DN (to the ESP32-S3 core sheet).

## 4. 3.3 V Regulator — Sheet 03_3V3_Power

This sheet turns the system supply node VSYS into the clean 3.3 V rail that everything intelligent runs on.

### U2 — low-dropout linear regulator (AP2112K-3.3, Diodes Inc., C51118, orderable code AP2112K-3.3TRG1)

LDO = Low-DropOut regulator: a linear regulator that makes a lower, steady voltage from a higher one by burning the difference off as heat, and that keeps regulating even when its input is only slightly above its output. Key numbers, all from its datasheet:

- Output: fixed 3.3 V. Guaranteed capable of at least 600 mA — a ceiling, not a constant output. The load draws what it draws; 600 mA is the most the regulator can supply while still holding 3.3 V.

- Dropout: ≈250 mV typical at heavy load. Meaning: input must stay above ≈3.55 V for a clean 3.3 V out.

- Enable pin (EN): has an internal 3 MΩ pull-down. Left unconnected, the regulator stays OFF. It is therefore tied directly to VIN so the regulator is always enabled. This tie is mandatory — a floating EN is a dead board.

- Protection built in: current limiting that folds back to ≈50 mA into a dead short, and thermal shutdown at ≈160 °C.

### Capacitors on this sheet

- C2 — 10 µF at +5V_PROT: the power-entrance bulk capacitor recommended by the Espressif guidelines. A local tank that steadies the 5 V side.

- C6 — 1 µF at the regulator input (VSYS). C7 — 1 µF at the regulator output. Both are the datasheet's specified stability capacitors.

### D2 — power LED (green, 0603) with R5 (10 kΩ, as built)

Lights whenever +3V3 is alive — on USB or on battery. At 10 kΩ it draws ≈120 µA — deliberately dim to protect the sleep budget; even so it is the largest single sleep-mode drain. Note for battery testing: D2 may be left unsoldered on boards used for battery-life measurements; ≈120 µA is large next to the ≈90 µA the rest of the board draws asleep.

### Behavior on battery — documented design decision

On battery power, VSYS equals the battery voltage (4.2 V full → 3.0 V empty). While the battery is above ≈3.6 V, the regulator holds a clean 3.3 V. Below that, the 3.3 V rail sags gracefully along with the battery; the ESP32-S3 keeps running (its supply range is 3.0–3.6 V) but Wi-Fi transmit margin shrinks near empty. This is accepted for Rev A: it is the same trade-off used by mainstream hobby boards of this class, the battery monitor reports the state, and the practical usable battery range is 4.2 V down to ≈3.5–3.6 V. A future revision may use a buck-boost converter (a switching regulator that holds 3.3 V across the whole battery range) if full-range operation is ever needed.

Thermal note: worst sustained dissipation in the regulator is about (4.6 V − 3.3 V) × 0.2 A ≈ 0.26 W. The layout gives U2 generous copper area as a heatsink (Section 9).

## 5. ESP32-S3 Core — Sheet 04_ESP32S3_Core

U3 is the ESP32-S3-WROOM-1-N8 (Espressif, C2913198): a module containing the ESP32-S3 dual-core processor, 8 MB of flash memory, the Wi-Fi/Bluetooth radio, the antenna, and the radio shielding — all pre-certified. Using the module instead of the bare chip removes the hardest parts of radio design from a first board. N8 decodes as: 8 MB quad-SPI flash, no PSRAM, −40 to +85 °C.

### Power and reset

- 3V3 (module pin 2): fed from +3V3 with C9 (22 µF bulk) and C10 (100 nF) placed at the pin. The module needs a supply able to deliver at least 0.5 A; its own worst peak is 355 mA during Wi-Fi transmit.

- EN (pin 3) is the chip-enable/reset input and must never float. It gets the standard reset network: R7 (10 kΩ) pull-up to +3V3, C8 (1 µF) to ground, and SW2 (RESET button) across the capacitor. The resistor-capacitor pair (an RC delay) holds the chip off for a few milliseconds while power stabilizes, exactly as the Espressif guidelines require.

- GND: module pins 1, 40, and the large exposed pad 41 all go to ground; the pad also pulls heat out of the module.

### Boot control and strapping pins

Strapping pins are pins the chip reads once, in the first instant after reset, to decide how to boot. Their state at that instant must be correct. The ESP32-S3's strapping pins are GPIO0, GPIO3, GPIO45, and GPIO46 (GPIO = General-Purpose Input/Output).

- GPIO0 (pin 27): 10 kΩ pull-up (R6) to +3V3 and SW1 (BOOT button) to ground. Held low during reset, it forces firmware-download mode. Per the guidelines: no large capacitor is allowed on this pin.

- GPIO3, GPIO45, GPIO46 (pins 15, 26, 24): left completely unconnected. Their internal defaults are correct; nothing may load them at boot.

- All other unused GPIO pins are left unconnected in this revision.

### USB connection

The ESP32-S3 has native USB: GPIO19 (pin 13) is USB D− and GPIO20 (pin 14) is USB D+. They connect to nets USB_DN and USB_DP from the USB sheet. Programming and the serial console run over this native USB — no separate USB-to-serial bridge chip exists on this board, and none is needed: the chip's built-in USB Serial/JTAG controller lets the flashing tool reset the chip into download mode in software. The BOOT and RESET buttons remain as the manual fallback for recovering from broken firmware.

### Complete pin map (every used module pin)

| Module pin | Pin name | Net | Role |
|---|---|---|---|
| 1, 40, 41      | GND / pad      | GND         | Ground and thermal pad.                                 |
| 2              | 3V3            | +3V3        | Module power input (C9 22 µF + C10 100 nF at the pin).  |
| 3              | EN             | EN          | Reset network R7 + C8 + SW2.                            |
| 13             | IO19           | USB_DN      | Native USB data minus.                                  |
| 14             | IO20           | USB_DP      | Native USB data plus.                                   |
| 23             | IO21           | SENS_PWR_EN | Drives the soil-probe power switch Q1 (low = probe on). |
| 27             | IO0            | BOOT        | Boot strap: R6 pull-up + SW1 to ground. TP10.           |
| 31             | IO38           | SDA         | I²C data to both sensors.                               |
| 32             | IO39           | SCL         | I²C clock to both sensors.                              |
| 36             | RXD0           | RXD0        | UART fallback receive. TP12.                            |
| 37             | TXD0           | TXD0        | UART fallback transmit. TP11.                           |
| 38             | IO2 (ADC1_CH1) | BAT_SENSE   | Battery voltage reading (half of VBAT).                 |
| 39             | IO1 (ADC1_CH0) | ADC_SOIL    | Soil probe analog reading.                              |

> **As-built note:** I²C moved to IO38/IO39 during capture — earlier drafts of this document said
> IO4/IO5. Every other unused GPIO is left unconnected.

ADC = Analog-to-Digital Converter, the circuit that turns a voltage into a number the firmware can read. Both analog inputs sit on ADC1, the converter Espressif recommends, and both use attenuation setting 3, whose usable range is 0–2900 mV. Each ADC pin gets a 100 nF capacitor to ground at the pin (C14 for soil, C17 for battery) per the guidelines.

## 6. Environmental Sensors — Sheet 05_I2C_Sensors

I2C (Inter-Integrated Circuit) is a two-wire shared bus: one clock line (SCL), one data line (SDA). Many chips share the same two wires; each has an address. Both lines idle high through pull-up resistors, and chips only ever pull the lines low — which is why the pull-ups are required.

- R8, R9 — 4.7 kΩ pull-ups, one per line, from the bus to +3V3. One pair serves the whole bus; adding more pairs would over-strengthen the pull and is a common beginner error.

### U4 — BME280 (Bosch, C92489): temperature, humidity, pressure

- Address 0x76, set by wiring its SDO pin to ground. SDO must never float — a floating SDO makes the address undefined.

- CSB pin wired to VDDIO (the 3.3 V rail): this permanently locks the chip into I2C mode instead of SPI mode.

- Decoupling: 100 nF at VDD (C11) and 100 nF at VDDIO (C12), per the datasheet.

- Placement cautions from the datasheet: the metal lid has a small vent hole that must never be covered (it is how pressure and humidity reach the sensing element), the package pin numbering runs clockwise in top view (opposite of most chips — checked during footprint work), and the part should sit away from heat sources (the regulator, the module) so it reads air temperature, not board temperature.

### U5 — VEML7700-TT (Vishay, C1850416): ambient light

- Address 0x10, fixed in silicon — no address pin. No conflict with the BME280's 0x76, so both share the bus happily.

- The -TT suffix is the top-view package: the light-sensing window faces up. It needs a clear view of the room and should sit away from the board's own LEDs.

- Decoupling: 100 nF at VDD (C13). Supply range 2.5–3.6 V — the 3.3 V rail fits.

## 7. Soil Probe Input — Sheet 06_Soil_Probe_Input

The soil probe (DFRobot SEN0193) is an external wand that plugs into the board. It outputs an analog voltage that falls as soil moisture rises. It is bought separately and never assembled onto the board.

- Probe specs, from its datasheet: supply 3.3–5.5 V, output 0–3.0 V, current ≈5 mA, connector interface PH2.0-3P.

### J2 — probe connector (JST S3B-PH-K-S(LF)(SN), C157929)

A 3-pin side-entry header from the JST PH family (2.0 mm pitch). 'PH2.0-3P' on the probe side means exactly this family, 3 positions — the mating plug housing is JST PHR-3. The family is rated 2 A, four hundred times the probe's needs. Pin assignment on the board (as built, matching the metered probe / DFRobot Gravity order): pin 1 = probe output (ADC_SOIL), pin 2 = SOIL_PWR — power on the middle pin, pin 3 = GND. (The originally planned order would have reverse-powered the probe.)

- Build-time check #1: the cable shipped with the probe has a PH plug on the probe end, but its other end is often a DuPont-style connector meant for Arduino headers. If so, a PH-to-PH 3-wire cable (about one dollar) replaces it.

- Build-time check #2: before soldering anything, buzz out the actual cable with a multimeter to confirm which wire lands on which J2 pad. Probe cables have no guaranteed color standard.

### Q1 — soil-probe power switch (AO3401A, C15127) with R10 (100 kΩ)

A P-channel MOSFET used as a high-side switch. MOSFET = Metal-Oxide-Semiconductor Field-Effect Transistor — here, simply a voltage-controlled switch. 'High-side' means it switches the power wire, not the ground wire. Its three pins: source (connected to +3V3), drain (connected to SOIL_PWR, the probe's power pin), gate (the control input, driven by net SENS_PWR_EN from GPIO21).

- Gate pulled high to +3V3 through R10 (100 kΩ): the default state is OFF, including during boot before firmware runs.

- Firmware drives the gate LOW to turn the probe ON, waits a moment for the reading to settle, samples the ADC, then drives the gate high again.

- Why it exists: the probe draws ≈5 mA continuously — about 120 mAh per day, enough to kill a 1000 mAh battery in roughly a week on its own. Switched, the probe costs almost nothing.

### The soil input, as built — direct connection, 100 kΩ pull-down, and C14 (100 nF) [2:1 divider removed 2026-07]

As built, the probe's output pin drives net ADC_SOIL into GPIO1 directly. R11 (100 kΩ) sits from ADC_SOIL to ground and C14 (100 nF) sits at the ESP32-S3 pin. The originally planned 2:1 divider (two equal series resistors halving the input) was deleted after bench measurement — see below.

- Why the divider was removed: bench measurement showed the probe tops out at ≈2.2 V from a 3.3 V supply (raw ≈2900 counts fully dry, ≈1465 in water, 12 dB attenuation) — already inside the ADC's 0–2900 mV range, so the halving stage added error without adding safety. A 100 kΩ pull-down to ground keeps the pin from floating while the probe is switched off, and even an accidental 5 V probe supply (output ≤3.0 V) stays below the pin's 3.6 V absolute maximum.

- C14 sits at the ESP32-S3 pin and steadies the reading, per the Espressif ADC guidance.

- Calibration plan: the raw numbers for 'dry' and 'in water' were recorded on the breadboard through the same direct (no-divider) chain the PCB uses — ≈2900 dry, ≈1465 in water — and must be re-recorded on the assembled board at bring-up. Note: at a 3.3 V supply the probe's real output span sits lower than the headline 0–3.0 V (that figure is at 5 V supply); calibration absorbs this completely.

## 8. Battery, Charger & Power Path — Sheet 07_Battery_PowerPath

This sheet does three jobs: protects against a backwards battery, charges the battery from USB, and automatically selects USB or battery as the source that actually runs the board.

### The battery itself (bought separately)

- Type: single-cell lithium-polymer (LiPo), '1S'. Nominal 3.7 V; 4.2 V full; ≈3.0 V empty.

- MUST have a built-in protection circuit (over-discharge / over-current cutoff) — the board deliberately has no battery-protection chip of its own. Note the protection is a last resort (disconnects at ≈2.5 V / ≈4.33 V); day-to-day limits are enforced by the charger (4.20 V) and by FIRMWARE: stop transmitting below ≈3.5 V (regulator headroom rule) and shut down at 3.0 V (battery floor). See Addendum A.

- Terminated in a JST-PH 2-pin plug, matching J3. Capacity 500–1200 mAh; the bench pack is a PKCell 503035 500 mAh (its protection IC is an ABLIC S-8261 per the pack drawing).

- First-connect ritual: measure the plug polarity with a multimeter before plugging in. LiPo leads have no universal color/polarity standard. Q2 makes a mistake survivable; the ritual makes it a non-event.

### J3 — battery connector (JST S2B-PH-K-S(LF)(SN), C173752)

The 2-pin side-entry sibling of J2, same JST PH family. Pin 1 = VBAT_RAW (battery positive), pin 2 = GND.

### Q2 — reverse-polarity protection (AO3401A, C15127)

A P-channel MOSFET wired as a one-way gate on the battery's positive line: drain to VBAT_RAW (the connector), source to VBAT (the protected rail), gate tied to ground.

- Battery correct: the MOSFET's internal body diode first lets current through, which raises the source voltage; the gate is then several volts below the source, the switch turns fully on, and current flows with only milliohms of loss — in BOTH directions, so charging current passes through it too.

- Battery reversed: the body diode faces the wrong way and blocks, and the gate-to-source voltage never turns the switch on. Nothing downstream ever sees the reversed voltage.

### U6 — battery charger (MCP73831T-2ACI/OT, Microchip, C424093)

A single-chip linear charge controller for one LiPo cell. The -2ACI code is the 4.20 V full-charge version in the SOT-23-5 package. It runs the industry-standard charge sequence (gentle preconditioning, constant current, constant voltage, automatic stop) with no firmware involvement.

- Powered from +5V_PROT (C15, 4.7 µF at its input). It only operates when USB is present; with USB gone it is unpowered and its built-in reverse blocking stops the battery from leaking back into it.

- Charge current is set by one resistor: R13 = 10 kΩ on the PROG pin gives ≈100 mA (the formula is I = 1000 V ÷ R). PROG must never be left floating. 100 mA is a gentle, safe rate for any cell 500 mAh and up.

- Output to VBAT with C16 (4.7 µF).

- D3 — charge LED (0603) with R12 (470 Ω, as built; ≈6 mA), wired from +5V_PROT through the LED into the STAT pin: the chip pulls STAT low while charging (LED on) and releases it when full (LED off). With no battery connected STAT floats and the LED may flicker faintly — expected, harmless.

### The automatic power path — D4, Q3, R16

This is the circuit that answers 'how does the board know to switch to the battery?' with physics, not decisions. Three parts:

- D4 — Schottky diode (SS14, MDD, C2480; SMA package, 1 A / 40 V). A Schottky diode is a one-way valve for current with an unusually small toll (≈0.4 V). It sits between +5V_PROT and VSYS. When USB is present, 5 V flows through it and VSYS sits at ≈4.6 V.

- Q3 — the source-select switch (AO3401A, C15127): a P-channel MOSFET with drain on VBAT, source on VSYS, and gate wired to +5V_PROT. R16 (10 kΩ, as built — changed from 100 kΩ after the 2026-07-20 review) pulls the gate to ground fast enough for a clean hand-over; it costs ≈0.5 mA only while USB is present.

- USB present: the gate sits at 5 V while the source sits at 4.6 V. The gate is ABOVE the source, so the switch is off, and Q3's internal diode also faces away from the higher VSYS — the battery is fully disconnected from the load. The board runs on USB; the charger has the battery all to itself.

- USB removed: the board's load drains +5V_PROT until Q3's body diode catches VSYS about 0.65 V below the battery; from that instant only R16 discharges the gate, so R16's value sets the hand-over speed. At 10 kΩ (as built) the body-diode phase lasts ≈50–100 ms, which the VSYS-side capacitors (C5, 10 µF and C18, 1 µF) and the processor's brown-out margin ride through; Q3 then turns fully on and VSYS becomes battery voltage with almost no loss. Correction to the original text: the capacitors on +5V_PROT do NOT bridge the gap — they sit behind D4 and actually hold Q3's gate up, delaying the switch; the bridging is done by the VSYS capacitors and the body diode. Full walkthrough: docs/Engineering_Notes.md, section 6. No firmware, no decisions — just two voltages racing and a switch that responds to their difference.

### Battery voltage monitor — R14, R15 (100 kΩ + 100 kΩ, 1%), C17 (100 nF)

An always-on voltage divider from VBAT to ground; the midpoint, net BAT_SENSE, is exactly half the battery voltage and goes to GPIO2 (ADC channel ADC1_CH1). A full battery reads 2.1 V — inside the ADC's 0–2900 mV range. The divider's constant drain is ≈21 µA, accepted as part of the ≈210 µA sleep floor. Firmware converts the reading to a percentage using a small lithium-cell voltage table.

## 9. Mechanical & Layout Rules — Sheet 08_Mechanical

This sheet holds the non-electrical realities: board outline, mounting, the antenna's demands, and the layout rules the electrical sheets impose.

- Board: **4 layers** (signal / GND / GND / signal on JLCPCB's JLC04161H-7628 stackup), 1.6 mm thick, 1 oz outer copper, lead-free HASL finish (HASL = Hot Air Solder Leveling), green solder mask. Two unbroken ground planes on the inner layers; no signals on the inner layers.

- Mounting: four M3 holes, one per corner, per the project plan.

- Antenna: the module's antenna end must protrude past the board edge (preferred) or sit at the edge with copper cut away beside and beneath it, at least 15 mm from any enclosure wall. No copper, no traces, no silkscreen under the antenna zone on any layer. Dense ground stitching vias surround (but never enter) the keep-out.

- USB data pair: routed as a short, tightly-coupled pair over the unbroken inner ground plane, with no vias. On the 4-layer stackup the pair geometry (0.29 mm trace / 0.20 mm gap over the 0.21 mm prepreg) lands at ≈90 Ω differential without paying for controlled-impedance fabrication; length-matched to 0.42 mm. The board is ordered as plain 4-layer — the geometry is right, the fab tolerance is simply not guaranteed, which is the correct trade at USB full speed.

- Regulator U2 gets a generous copper pour on its tab/ground for heat spreading (worst case ≈0.26 W sustained).

- BME280 placed away from U2 and U3 (heat), lid vent hole kept exposed. VEML7700 placed with a clear sky view, away from D2/D3.

- The two ADC traces (ADC_SOIL, BAT_SENSE) routed short, away from the USB pair and the antenna.

- Fuse F1, TVS D1, and ESD chip U1 all placed immediately at the USB connector — protection parts protect nothing if the zap passes them by on the way in.

- Trace widths: power nets (USB_VBUS, +5V_PROT, VSYS, VBAT, +3V3) at least 0.5 mm; signals 0.2–0.25 mm; verified with the Saturn PCB Toolkit against IPC-2221 before layout is finalized.

## 10. Complete Bill of Materials (BOM)

BOM = Bill of Materials: the complete shopping list the assembler builds from. JLCPCB part numbers ('C' numbers) are the definitive identifiers — search these on jlcpcb.com/parts. All C-numbered parts below were verified in stock and assemblable during design. Passive values are final; their exact catalog numbers are chosen from JLCPCB's Basic library (their always-stocked core catalog) at BOM upload.

### Semiconductors and modules

| Ref | What it is | Exact part | JLC # | Library | Job on the board |
|-----|------------|-----------|-------|---------|------------------|
| U1  | USB data-line ESD protection array | USBLC6-2SC6 (ST) | C7519 | Extended | Clamps static zaps on D+/D−. |
| U2  | 3.3 V low-dropout linear regulator (LDO) | AP2112K-3.3TRG1 | C51118 | Extended | Makes the +3V3 rail from VSYS. |
| U3  | Wi-Fi microcontroller module | ESP32-S3-WROOM-1-N8 | C2913198 | Extended | The brain: firmware, sensors, USB, Wi-Fi. **Standard-PCBA tier only.** |
| U4  | Temp / humidity / pressure sensor (I²C) | BME280 (Bosch) | C92489 | Extended | Environment readings, address 0x76. **Standard-PCBA tier only.** |
| U5  | Ambient light sensor (I²C) | VEML7700-TT (Vishay) | C1850416 | Extended | Light level, address 0x10. |
| U6  | 1-cell LiPo linear charge controller | MCP73831T-2ACI/OT | C424093 | Extended | Charges the battery at ≈100 mA from USB. |
| Q1  | P-channel MOSFET (switch) | AO3401A | C15127 | Basic | Soil-probe power switch (high side). |
| Q2  | P-channel MOSFET (switch) | AO3401A | C15127 | Basic | Battery reverse-polarity protection. |
| Q3  | P-channel MOSFET (switch) | AO3401A | C15127 | Basic | USB/battery source-select switch. |
| D1  | TVS surge-protection diode, 5 V | SMF5.0A | C2980403 | Extended | Clamps spikes on the USB power line. |
| D2  | LED, yellow-green, 0603 | KT-0603YG | C2289 | Extended | Power indicator on +3V3 (Vf 2.0–2.2 V — the right chemistry for a 3.3 V rail). |
| D3  | LED, red, 0603 | KT-0603R | C2286 | Basic | Charging indicator on the charger's STAT pin. |
| D4  | Schottky diode, 1 A / 40 V, SMA | SS14 (MDD) | C2480 | Basic | Feeds VSYS from USB. |

### Connectors, protection, switches

| Ref | What it is | Exact part | JLC # | Library | Job on the board |
|-----|------------|-----------|-------|---------|------------------|
| J1  | USB-C receptacle, 16-pin, USB 2.0 | TYPE-C-31-M-12 | C165948 | Extended | Power in + programming/data. Small per-piece handling fee at JLC. |
| J2  | JST PH 3-pin side-entry header | S3B-PH-K-S(LF)(SN) | C157929 | Verify | Soil probe. Pin 1 = signal, pin 2 = power, pin 3 = GND — **buzz out the cable first.** |
| J3  | JST PH 2-pin side-entry header | S2B-PH-K-S(LF)(SN) | C173752 | Verify | Battery. Pin 1 = +. **Meter plug polarity before first connect.** |
| F1  | Resettable PPTC fuse, 0.75 A hold | 1206L075/16WR | C371166 | Verify | Current ceiling on the 5 V input. |
| SW1 | Tactile switch, SMD 5.1 × 5.1 mm | TS-1187A-B-A-B | C318884 | Basic | **BOOT** button. Project footprint, pads renumbered 1/1/2/2. |
| SW2 | Tactile switch, SMD 5.1 × 5.1 mm | TS-1187A-B-A-B | C318884 | Basic | **RESET** button. |

### Passive components (resistors and capacitors, 0603 unless noted)

| Ref | Value | JLC # | Purpose |
|-----|-------|-------|---------|
| R1, R2 | 22 Ω | C23345 | USB data series resistors, placed near the module. |
| R3, R4 | 5.1 kΩ | C23186 | USB-C CC pull-downs — one per CC pin; announces the board as a 5 V sink. |
| R5 | 10 kΩ | C98220 | Power-LED (D2) current limit — ≈120 µA, deliberately dim. |
| R6 | 10 kΩ | C98220 | GPIO0 (boot) pull-up to +3V3. |
| R7 | 10 kΩ | C98220 | EN (reset) pull-up to +3V3. |
| R8, R9 | 4.7 kΩ | C23162 | I²C bus pull-ups, one pair for the whole bus. |
| R10 | 100 kΩ | C25803 | Q1 gate pull-up — soil probe stays OFF until firmware says otherwise. |
| R11 | 100 kΩ | C25803 | ADC_SOIL pull-down — holds the pin steady while the probe is unpowered. |
| R12 | 470 Ω | C23179 | Charge-LED (D3) current limit, ≈6 mA. |
| R13 | 10 kΩ | C98220 | Charger PROG resistor → ≈100 mA charge current. |
| R14, R15 | 100 kΩ 1% | C25803 | Battery-sense 2:1 divider to GPIO2. |
| R16 | 10 kΩ | C98220 | Q3 gate pull-down — makes the battery take over when USB leaves. 10 kΩ (was 100 kΩ) for a fast hand-over. |
| C1 | 100 nF | C14663 | U1 (ESD chip) VBUS-pin decoupling. |
| C2 | 10 µF, 25 V (0805) | C15850 | Power-entrance bulk on +5V_PROT. |
| C3, C4 | 100 nF — **DNP** | C14663 | Reserved USB tuning capacitors. Footprints only, not assembled. |
| C5 | 10 µF, 25 V (0805) | C15850 | VSYS bulk — half of what bridges the USB→battery hand-over. |
| C6 | 1 µF | C15849 | Regulator input stability capacitor (VSYS). |
| C7 | 1 µF | C15849 | Regulator output stability capacitor (+3V3). |
| C8 | 1 µF | C15849 | EN reset-delay capacitor. |
| C9 | 22 µF, 25 V (0805) | C45783 | Bulk at the module's 3V3 pin. |
| C10 | 100 nF | C14663 | Module bypass at the 3V3 pin. |
| C11, C12 | 100 nF | C14663 | BME280 VDD and VDDIO decoupling. |
| C13 | 100 nF | C14663 | VEML7700 decoupling. |
| C14 | 100 nF | C14663 | ADC filter at the soil input pin. |
| C15, C16 | 4.7 µF | C19666 | Charger input / output capacitors. |
| C17 | 100 nF | C14663 | ADC filter at the battery-sense pin. |
| C18 | 1 µF | C15849 | VSYS decoupling — the other half of the hand-over bridge. |

> Passive JLC numbers are the ones **actually consumed on the 2026-08-21 order**. Basic-tier staples
> cycle in and out of stock weekly — re-verify every line on the JLC matching page at order time.
> The 10 kΩ line moved from C25804 to **C98220** after a JLC pre-order failed on 2026-08-18.

### Not assembled — bought separately

- DFRobot SEN0193 capacitive soil probe (plugs into J2). Sold by DigiKey/DFRobot/Amazon. Not waterproof past its marked line.

- 1S protected LiPo battery, 3.7 V nominal, JST-PH plug, 1000 mAh class (plugs into J3). The cell used for Rev A bring-up is a 500 mAh PKCell LP503035, so halve any 1000 mAh runtime estimate.

- If the probe's included cable ends in a DuPont-style connector: one PH-to-PH 3-pin cable.

## 11. Test Points

TP = Test Point: a bare copper pad, labeled on the silkscreen, where a multimeter or oscilloscope probe lands. They cost nothing and make bring-up and debugging enormously faster.

| TP | Net | What I check there |
|----|-----|----------------------|
| TP1  | +5V_PROT  | 5 V present after the fuse. |
| TP2  | VSYS      | System node — watch the USB↔battery hand-over here. |
| TP3  | +3V3      | Regulator output alive and at 3.3 V. |
| TP4  | VBAT      | Protected battery rail; charge-voltage check. |
| TP5  | BAT_SENSE | Half-battery voltage into the ADC. |
| TP6  | ADC_SOIL  | Soil-probe voltage at the ADC pin. |
| TP7  | GND       | Meter/scope ground reference. |
| TP8  | GND       | Second ground reference — scope probe spring, opposite corner. |
| TP9  | EN        | Reset line high; reset-button action. |
| TP10 | BOOT      | Boot strap state; boot-button action. |
| TP11 | TXD0      | UART fallback transmit (recovery flashing / logs). Through-hole. |
| TP12 | RXD0      | UART fallback receive. Through-hole. |

TP11, TP12 and the adjacent GND pad are through-hole — the UART recovery trio, so a header can be
soldered in if USB is ever bricked. All other test points are 1.5 mm bare pads.

> **As-built note:** earlier drafts of this document planned TP1–TP15, including probe points on the
> USB pair and the I²C lines. The board carries **TP1–TP12** as listed above; the USB and I²C test
> points were dropped to keep the pair short and the antenna keep-out clear. TP3, TP5 and TP9 sit
> inside the self-imposed 15 mm antenna ring — waived for Rev A with probe-lead discipline during
> RF-active tests (see `docs/PROJECT_STATUS.md`, *Waivers on record*).

## 12. Firmware Plan (summary)

- Wake on a timer (deep sleep between cycles). Deep sleep = the processor mostly off, only a tiny wake-up clock running.

- On wake: drive GPIO21 low → soil probe powers up; wait ≈200 ms to settle; read ADC_SOIL (average several samples); drive GPIO21 high → probe off.

- Read BME280 (temperature, humidity, pressure) and VEML7700 (light) over I2C.

- Read BAT_SENSE, double it, convert to percentage via a lithium voltage table.

- Join Wi-Fi, publish the readings (MQTT or HTTP), disconnect, return to deep sleep.

- Soil calibration constants (dry value, wet value) are compiled in from the breadboard measurements taken through the identical direct, no-divider chain (≈2900 dry / ≈1465 in water).

- Serial console and flashing over native USB; TXD0/RXD0 test points are the recovery path if firmware ever bricks USB.

## 13. Bring-Up Plan (first power, in order)

- 1. Visual inspection against the schematic: orientations of U3–U5, D2–D4, Q2–Q3, electrolytic-style markings, no bridges.

- 2. Meter checks, no power: no short between GND and +5V_PROT, VSYS, +3V3, VBAT.

- 3. USB in through a current-limited source if available. Check TP1 ≈5 V, TP2 ≈4.6 V, TP3 = 3.3 V. Power LED on.

- 4. Enumeration: the board appears as a USB serial device. Flash a blink/hello firmware.

- 5. I2C scan: devices found at 0x76 and 0x10.

- 6. Battery (polarity verified with a meter first!) into J3, USB still in: charge LED on, TP4 climbs toward 4.2 V, charge current ≈100 mA.

- 7. The hand-over test: board running, scope or meter on TP3 — unplug USB. The board must keep running; TP2 snaps to battery voltage; TP3 stays at 3.3 V.

- 8. Battery-only boot test: power up from battery alone.

- 9. Probe in J2 (after cable buzz-out): dry reading and water reading recorded; compare to breadboard constants.

- 10. Sleep-current measurement in series with the battery: expect ≈210 µA floor as built (≈90 µA with D2, the power LED, unsoldered).

- 11. Wi-Fi range sanity check and a 24-hour soak run.

## 14. Manufacturing & Assembly

- Fab + assembly at JLCPCB: **4-layer PCB, Standard PCBA**, top side only, SMT assembly plus through-hole service for the two JST connectors. SMT = Surface-Mount Technology (parts sit on the surface); THT = Through-Hole Technology (leads pass through the board). Standard tier is forced by two parts, not by choice: the ESP32-S3 module (C2913198) and the BME280 (C92489) are both Standard-only.

- Most ICs are 'Extended' catalog parts, each carrying a one-time feeder-loading fee; the AO3401A, SS14, and most passives come from the free 'Basic' library. **Actual Rev A run (5 boards):** setup $25.56 · stencil $8.21 · feeder loading $38.25 (13 Extended lines) · SMT $2.04 · hand-soldered J2/J3 $4.04 · X-ray on the module pad $8.20 · packaging $0.49 — PCBA $86.79, PCB $12.10, components $0.00 (all 27 unique parts consumed from a pre-stocked JLC Parts Library). See `fabrication/revA/ORDER_NOTES.md` §7.

- The USB-C connector carries a small per-piece special-handling fee at JLC — expected.

- If any part is ever out of stock at order time: JLCPCB's Global Sourcing / pre-order service buys the identical part from DigiKey, Mouser, Arrow, or Verical into the account's parts library for assembly — the design never changes. Consignment (shipping parts to JLC) is the further fallback.

- The soil probe and the battery are never part of the assembly order.

## 15. Known Limitations & Accepted Risks (stated openly)

- Regulator sag on a low battery: below ≈3.6 V battery, the 3.3 V rail follows the battery down. Accepted for Rev A; battery monitor reports it; buck-boost is the Rev B path if ever needed.

- Linear regulator heat: ≈0.26 W sustained worst case — handled with copper pour; verified at bring-up.

- Fuse derating: the 0.75 A hold falls to 0.65 A at 40 °C; margins were sized against the derated number.

- USB differential impedance: the pair is at the ≈90 Ω geometry over the inner ground plane, but the board is ordered as plain 4-layer, so the impedance is designed-for rather than fab-guaranteed. Correct trade at USB full speed; controlled-impedance fabrication is available if a future revision ever needs the guarantee.

- Probe output span at 3.3 V supply is smaller than the headline 0–3.0 V figure; the calibration procedure absorbs it.

- The probe is not waterproof past its marked line; the board itself is not weatherproof.

- Charge LED may flicker faintly with no battery connected (charger status pin floats) — cosmetic only.

- The board has no on-board battery-protection chip; the REQUIREMENT to use protected cells covers it.

## 16. Build-Time Checklist

*Status as of 2026-08-23. Rev A is ordered; the items below are what the build still owes.*

**Still open — for the bench, when the boards land:**

- [ ] Buzz out the soil-probe cable wire order before soldering / first use.
- [ ] Confirm the probe cable's board-end connector; buy a PH-to-PH cable if needed.
- [ ] Meter-check battery plug polarity before first connection. The charge LED does **not** prove polarity.
- [ ] Re-record the soil calibration constants (dry / in water) on the assembled board.

**Closed during design:**

- [x] Basic-library catalog numbers picked for every passive, LED and tactile switch — see the BOM tables in Section 10, and `fabrication/revA/BOM-ESP32S3_PlantMonitor.csv` for what was actually ordered.
- [x] SS14 Schottky datasheet added — `references/datasheets/DIODE_SS14_Schottky_1A40V_Family_Datasheet.pdf`.
- [x] SMF5.0A TVS datasheet added — `references/datasheets/TVS_MDD_SMF5_0A_5V_Unidirectional_Datasheet.pdf`.
- [x] Duplicate SEN0193 file consolidated — `references/datasheets/SENSOR_DFRobot_SEN0193_Capacitive_Soil_Moisture_Module_Web_Datasheet.pdf`.

## Addendum A — Renumbering & As-Built Record

This document was written before schematic capture. During capture and the 2026-07-20 design review
the board picked up KiCad's own reference designators, which did not match the planning-stage
designators used in the original draft. **On 2026-08-23 the body of this document was renumbered to
the as-built designators**, so the text, the schematic, the schematic PDF and the ordered BOM now
agree. This addendum keeps the record of what moved and what changed electrically.

### Designator renumbering applied to this document

| Planning draft | As built | Part |
|---|---|---|
| U1 | **U3** | ESP32-S3-WROOM-1 module |
| U2 | **U1** | USBLC6-2SC6 USB ESD array |
| U3 | **U2** | AP2112K-3.3 LDO |
| U4 | **U6** | MCP73831 charger |
| U5 | **U4** | BME280 |
| U6 | **U5** | VEML7700 |
| Q1 | **Q2** | Reverse-polarity guard |
| Q2 | **Q1** | Soil-probe power switch |
| Q3 | Q3 | Source-select switch |
| R1, R2 | **R4, R3** | USB-C CC pull-downs |
| R3, R4 | **R1, R2** | USB 22 Ω series |
| R5 | **R7** | EN pull-up |
| R6 | R6 | IO0 pull-up |
| R7, R8 | **R8, R9** | I²C pull-ups |
| R9, R10 | **deleted** | The removed soil 2:1 divider |
| — | **R11** | New: 100 kΩ ADC_SOIL pull-down that replaced the divider |
| R11 | **R13** | Charger PROG |
| R12 | R12 | Charge-LED limit, 470 Ω |
| R13 | **R5** | Power-LED limit, 10 kΩ |
| R14 | **R16** | Q3 gate pull-down, 10 kΩ |
| R15 | **R10** | Q1 gate pull-up |
| R16, R17 | **R14, R15** | Battery-sense divider |
| D1 | **D2** | Power LED |
| D2 | **D3** | Charge LED |
| D3 | **D1** | TVS |
| D4 | D4 | Power-path Schottky |
| SW1 | **SW2** | RESET button |
| SW2 | **SW1** | BOOT button |
| C1 | **C2** | +5V_PROT bulk |
| C2, C3 | **C6, C7** | Regulator in / out |
| C4 | **C9** | Module bulk |
| C5 | **C10** | Module bypass |
| C6 | **C8** | EN delay |
| C7, C8 | **C3, C4** | Reserved USB caps (DNP) |
| C9 | **C1** | ESD-chip VBUS decoupling |
| C10, C11 | **C11, C12** | BME280 decoupling |
| C12 | **C13** | VEML7700 decoupling |
| C13 | **C14** | Soil ADC filter |
| C14, C15 | **C15, C16** | Charger in / out |
| C16 | **C17** | Battery-sense ADC filter |
| — | **C5, C18** | New: 10 µF + 1 µF VSYS bulk (added after the review) |

The I²C pull-up pair (planning R7/R8 → as-built R8/R9) was missing from the original cross-map and
was recovered from the schematic netlist. Test points were re-derived the same way: the planning draft
listed TP1–TP15, the board carries TP1–TP12, and the mapping is in Section 11.

### Electrical changes made after the 2026-07-20 review

1. **Soil 2:1 divider removed.** The probe drives the ADC directly; R11 (100 kΩ) pulls the pin down and C14 (100 nF) filters it. Bench measurement showed the probe tops out at ≈2.2 V from a 3.3 V supply — already inside the ADC's 0–2900 mV window, so the halving stage added error without adding safety.
2. **J2 pin order** is signal / power / GND, matching the metered DFRobot probe. The originally planned order would have reverse-powered it.
3. **Power-LED resistor is 10 kΩ** (≈120 µA) — deliberately dim, to protect the sleep budget.
4. **Charge-LED resistor is 470 Ω** (≈6 mA).
5. **10 µF added on VSYS (C5).** The body text's original claim that the +5V_PROT capacitors bridge the hand-over was wrong and is corrected in Section 8: those capacitors sit behind D4 and actually hold Q3's gate *up*, delaying the switch. The bridging is done by the VSYS capacitors and Q3's body diode.
6. **Q3 gate pull-down is 10 kΩ**, changed from 100 kΩ, so the USB-unplug hand-over completes in ≈50–100 ms instead of ≈1 s — this is what prevents brown-out resets at mid battery.
7. **USB-C GND pins A1/B1/A12/B12** are explicitly tied to ground (an early capture left them floating).
8. **I²C moved to IO38/IO39** from the planned IO4/IO5.
9. **Board went from 2 layers to 4** (signal / GND / GND / signal, JLC04161H-7628). Sections 9, 14 and 15 are written to the 4-layer build.

### Firmware protection duties

Hardware only reports; firmware must act. Stop Wi-Fi transmission below ≈3.5 V battery (the AP2112K
needs ≈200 mV headroom at the 355 mA TX peak — a regulator rule); enter permanent deep sleep at
3.0 V (the PKCell floor — a battery rule). Deep sleep still draws ≈210 µA as built (≈90 µA with the
power LED removed): about 100 days from full to the pack protector's 2.5 V disconnect on the 500 mAh
bench cell. **Never store the board with a battery attached** — the pack cannot be recovered from 0 V.

### Companion documents

| Document | What it carries |
|---|---|
| [`PROJECT_STATUS.md`](PROJECT_STATUS.md) | The live hand-off: current state, waivers, standing facts |
| [`BringUp_Guide.md`](BringUp_Guide.md) | First-power procedure, expected voltages, probe points |
| [`PinMap_CheatSheet.md`](PinMap_CheatSheet.md) | GPIO / net / connector map, as built |
| [`Engineering_Notes.md`](Engineering_Notes.md) | Bench notes, battery-protection analysis, hand-over walkthrough |
| [`RevB_Upgrade_Plan.md`](RevB_Upgrade_Plan.md) | Ranked upgrades, integrated power path first |
| [`reviews/Design_Review_RevA_2026-07-20.md`](reviews/Design_Review_RevA_2026-07-20.md) | The full pre-fab review record this addendum came from |
| [`../fabrication/revA/ORDER_NOTES.md`](../fabrication/revA/ORDER_NOTES.md) | Every setting the board was ordered with, and the as-ordered cost record |
