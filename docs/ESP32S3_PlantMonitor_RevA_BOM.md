# ESP32-S3 Plant Monitor — Rev A Bill of Materials

*GENERATED FROM THE SCHEMATIC 2026-07-20 (53 parts / 29 lines; LCSC fields verified in-file). The schematic is the master — regenerate this sheet after any change. Library status (Basic/Extended) and stock are LIVE values: re-verify every line on jlcpcb.com/parts at order time. Test points & mounting holes intentionally excluded (bare copper / holes).*

> The spreadsheet this table is generated from lives beside it as [`ESP32S3_PlantMonitor_RevA_BOM.xlsx`](ESP32S3_PlantMonitor_RevA_BOM.xlsx). For what was **actually ordered** on 2026-08-21 — including the two substitutions made on order day — see [`fabrication/revA/BOM-ESP32S3_PlantMonitor.csv`](../fabrication/revA/BOM-ESP32S3_PlantMonitor.csv).

| # | Category | Designators | Qty | Value | MPN | LCSC # | Footprint (package) | Library | Populate | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 1 ICs & module | U1 | 1 | USBLC6-2SC6 | USBLC6-2SC6 | C7519 | SOT-23-6 | Extended | Yes |  |
| 2 | 1 ICs & module | U2 | 1 | AP2112K-3.3 | AP2112K-3.3TRG1 | C51118 | SOT-23-5 | Extended | Yes | 3.3 V / 600 mA |
| 3 | 1 ICs & module | U3 | 1 | ESP32-S3-WROOM-1 | ESP32-S3-WROOM-1-N8 | C2913198 | ESP32-S3-WROOM-1 | Extended | Yes | Standard PCBA tier only (module size) |
| 4 | 1 ICs & module | U4 | 1 | BME280 | BME280 | C92489 | Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering | Extended | Yes | 0x76; keep vent clear, away from heat |
| 5 | 1 ICs & module | U5 | 1 | VEML7700-TT | VEML7700-TT | C1850416 | SENSOR-SMD_EML7700-TT | Extended | Yes | -TT = top-view window; 0x10 |
| 6 | 1 ICs & module | U6 | 1 | MCP73831-2-OT | MCP73831T-2ACI/OT | C424093 | SOT-23-5 | Extended | Yes | 4.20 V variant; PROG 10k -> ~100 mA |
| 7 | 2 Transistors | Q1, Q2, Q3 | 3 | AO3401A | AO3401A | C15127 | SOT-23 | Basic | Yes | Q1 soil switch / Q2 reverse guard / Q3 source select |
| 8 | 3 Diodes & LEDs | D1 | 1 | SMF5.0A | SMF5.0A | C2980403 | D_SOD-123F | Extended | Yes | TVS on USB_VBUS |
| 9 | 3 Diodes & LEDs | D2 | 1 | LED_YG | KT-0603YG | C2289 | LED_0603_1608Metric | Extended | Yes | Yellow-green Vf 2.0-2.2 V (correct chemistry for 3.3 V rail). 30-42 mcd -> dim at 120 uA by design; bench-check. Went Extended 2026-07. |
| 10 | 3 Diodes & LEDs | D3 | 1 | LED_RED | KT-0603R | C2286 | LED_0603_1608Metric | Basic | Yes | Red, ~6 mA from STAT |
| 11 | 3 Diodes & LEDs | D4 | 1 | SS14 | SS14 | C2480 | D_SMA | Basic | Yes | Power-path Schottky |
| 12 | 4 Connectors & electromech | F1 | 1 | 1206L075/16WR | 1206L075/16WR | C371166 | Fuse_1206_3216Metric | Verify | Yes | 0.75 A hold / 1.5 A trip / 16 V |
| 13 | 4 Connectors & electromech | J1 | 1 | USB_C_Receptacle_USB2.0_16P | TYPE-C-31-M-12 | C165948 | USB_C_Receptacle_HRO_TYPE-C-31-M-12 | Extended | Yes | KiCad ships exact footprint. Small handling fee. |
| 14 | 4 Connectors & electromech | J2 | 1 | Conn_01x03 | S3B-PH-K-S(LF)(SN) | C157929 | JST_PH_S3B-PH-K_1x03_P2.00mm_Horizontal | Verify | Yes | Soil: pin1 sig / pin2 pwr / pin3 GND. Buzz out cable. |
| 15 | 4 Connectors & electromech | J3 | 1 | Conn_01x02 | S2B-PH-K-S(LF)(SN) | C173752 | JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal | Verify | Yes | Battery: pin1 = +. METER PLUG POLARITY FIRST. |
| 16 | 4 Connectors & electromech | SW1, SW2 | 2 | SW_Push | TS-1187A-B-A-B | C318884 | SW-SMD_4P-L5.1-W5.1-P3.70-LS6.5-TL_H1.5 | Basic | Yes | Project footprint, pads renumbered 1/1/2/2 (verified vs datasheet A-B/C-D pairing) |
| 17 | 5 Resistors 0603 | R1, R2 | 2 | 22 | 0603WAF220JT5E | C23345 | R_0603_1608Metric | Basic | Yes | USB series |
| 18 | 5 Resistors 0603 | R10, R11 | 2 | 100k | 0603WAF1003T5E | C25803 | R_0603_1608Metric | Basic | Yes | All +-1% |
| 19 | 5 Resistors 0603 | R12 | 1 | 470 | 0603WAF4700T5E | C23179 | R_0603_1608Metric | Basic | Yes | Charge-LED limit ~6 mA |
| 20 | 5 Resistors 0603 | R13, R16, R5, R6, R7 | 5 | 10k | RC0603FR-0710KL | C98220 | R_0603_1608Metric | Extended | Yes | Stock was 0 on 2026-07-20 - re-check; alternate: YAGEO RC0603FR-0710KL = C98220 (Extended) — was C25804 (Uni-Royal, Basic) — JLC pre-order failed 2026-08-18, switched to Yageo C98220 |
| 21 | 5 Resistors 0603 | R14, R15 | 2 | 100k 1% | 0603WAF1003T5E | C25803 | R_0603_1608Metric | Basic | Yes | All +-1% |
| 22 | 5 Resistors 0603 | R3, R4 | 2 | 5.1k | 0603WAF5101T5E | C23186 | R_0603_1608Metric | Basic | Yes | CC pull-downs, one per pin |
| 23 | 5 Resistors 0603 | R8, R9 | 2 | 4.7k | 0603WAF4701T5E | C23162 | R_0603_1608Metric | Basic | Yes | I2C pull-ups |
| 24 | 6 Capacitors | C1, C10, C11, C12, C13, C14, C17 | 7 | 100nF | CC0603KRX7R9BB104 | C14663 | C_0603_1608Metric | Basic | Yes |  |
| 25 | 6 Capacitors | C15, C16 | 2 | 4.7uF | CL10A475KO8NNNC | C19666 | C_0603_1608Metric | Basic | Yes | 16 V X5R — charger in/out |
| 26 | 6 Capacitors | C18, C6, C7, C8 | 4 | 1uF | CL10A105KB8NNNC | C15849 | C_0603_1608Metric | Basic | Yes | 50 V X5R |
| 27 | 6 Capacitors | C2, C5 | 2 | 10uF | CL21A106KAYNNNE | C15850 | C_0805_2012Metric | Basic | Yes | 25 V X5R — 5 V-rail & VSYS bulk (alt: Murata C440198) |
| 28 | 6 Capacitors | C3, C4 | 2 | 100nF | CC0603KRX7R9BB104 | C14663 | C_0603_1608Metric | Basic | No (DNP) | DNP - footprint only, not assembled. |
| 29 | 6 Capacitors | C9 | 1 | 22uF | CL21A226MAQNNNE | C45783 | C_0805_2012Metric | Basic | Yes | 25 V X5R — module bulk |

## Not assembled by JLCPCB — to buy and attach myself

| Item | Spec | Plugs into | Where | Notes |
|---|---|---|---|---|
| DFRobot SEN0193 soil probe | 3.3-5.5 V, out 0-3.0 V (~2.2 V max @3.3 V), ~5 mA | J2 | DigiKey/DFRobot/Amazon | Buzz out cable order first. Calibration (direct chain): wet ~1465 / dry ~2900 raw. |
| 1S protected LiPo | 3.7 V, JST-PH; bench cell: PKCell LP503035 500 mAh | J3 | Battery retailer | PCM required. METER PLUG POLARITY BEFORE FIRST CONNECT - charge LED does NOT prove polarity. Never store with battery attached. |
| PH-to-PH 3-pin cable (if needed) | JST PH 2.0 | J2 | ~$1 generic | Only if probe cable ends DuPont-style. |
