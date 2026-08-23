# Footprint Check — ESP32-S3 Plant Monitor Rev A

*20 packages · 69 placements · KiCad 10 · JLCPCB assembly. Verified before layout; frozen here as the Rev A record.*

Every footprint below was checked pad-by-pad against the datasheet linked in its row — pad size and pitch, courtyard, pin-1 marking, and (for the parts JLC places) the orientation the assembler's preview would show. Two footprints are project-custom and are marked **⚠ Custom**; the rest are KiCad standard library.

## Summary

| Package | Footprint | Library | Category | Placements | Datasheet |
|---|---|---|---|---|---|
| MLCC 0603 (1608 metric) | `Capacitor_SMD:C_0603_1608Metric` | KiCad std | Passive | 15 | [Samsung MLCC](../references/datasheets/CAP_Samsung_MLCC_Catalogue_2015-11.pdf) |
| MLCC 0805 (2012 metric) | `Capacitor_SMD:C_0805_2012Metric` | KiCad std | Passive | 3 | [Samsung MLCC](../references/datasheets/CAP_Samsung_MLCC_Catalogue_2015-11.pdf) |
| Chip resistor 0603 (1608 metric) | `Resistor_SMD:R_0603_1608Metric` | KiCad std | Passive | 16 | — |
| Chip LED 0603 | `LED_SMD:LED_0603_1608Metric` | KiCad std | Opto | 2 | [XINGLIGHT XL-0603QYGC](../references/datasheets/LED_XINGLIGHT_XL-0603QYGC_YellowGreen_0603_C2289_Datasheet.pdf) · [KENTO KT-0603R](../references/datasheets/LED_KENTO_KT-0603R_Red_0603_C2286_Datasheet.pdf) |
| SMF5.0A TVS diode | `Diode_SMD:D_SOD-123F` | KiCad std | Protection | 1 | [MDD SMF5](../references/datasheets/TVS_MDD_SMF5_0A_5V_Unidirectional_Datasheet.pdf) |
| SS14 Schottky 1A/40V | `Diode_SMD:D_SMA` | KiCad std | Power | 1 | [SS14 Schottky](../references/datasheets/DIODE_SS14_Schottky_1A40V_Family_Datasheet.pdf) |
| Resettable PPTC fuse 1206 | `Fuse:Fuse_1206_3216Metric` | KiCad std | Protection | 1 | [Littelfuse 1206L](../references/datasheets/FUSE_Littelfuse_1206L_Resettable_PPTC_Datasheet_2024.pdf) |
| HRO USB-C receptacle, 16P | `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` | KiCad std | Connector | 1 | [TYPE-C-31-M-12 Receptacle](../references/datasheets/USBC_TYPE-C-31-M-12_Receptacle_C165948_Footprint_Drawing.pdf) |
| SOT-23-6 — ESD protection | `Package_TO_SOT_SMD:SOT-23-6` | KiCad std | IC | 1 | [ST USBLC6-2SC6](../references/datasheets/ESD_ST_USBLC6-2SC6_USB2_DataLine_Protection_Datasheet.pdf) |
| SOT-23-5 — LDO + LiPo charger | `Package_TO_SOT_SMD:SOT-23-5` | KiCad std | IC | 2 | [Diodes AP2112K-3.3](../references/datasheets/LDO_Diodes_AP2112K-3.3_600mA_3V3_Regulator_Datasheet.pdf) · [Microchip MCP73831](../references/datasheets/CHARGER_Microchip_MCP73831_LiPo_Linear_Charger_Datasheet_DS20001984H.pdf) |
| SOT-23 — P-channel MOSFET | `Package_TO_SOT_SMD:SOT-23` | KiCad std | Discrete | 3 | [AlphaOmega AO3401A](../references/datasheets/MOSFET_AlphaOmega_AO3401A_PchannelMOSFET_Datasheet.pdf) |
| ESP32-S3-WROOM-1 module | `RF_Module:ESP32-S3-WROOM-1` | KiCad std | Module | 1 | [Espressif WROOM-1](../references/datasheets/ESP32S3_Espressif_WROOM-1_WROOM-1U_Module_Datasheet_v1.8.pdf) |
| Bosch LGA-8 — BME280 | `Package_LGA:Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering` | KiCad std | Sensor | 1 | [Bosch BME280](../references/datasheets/SENSOR_Bosch_BME280_Temperature_Humidity_Pressure_Datasheet_Rev1.24_2024.pdf) |
| Vishay VEML7700-TT light sensor | `PlantMonitor_JLC:SENSOR-SMD_EML7700-TT` | ⚠ Custom | Sensor | 1 | [Vishay VEML7700](../references/datasheets/SENSOR_Vishay_VEML7700_Ambient_Light_I2C_Datasheet_Rev1.8_2024.pdf) |
| SMD tactile switch 5.1 × 5.1 mm (XKB TS-1187A) | `PlantMonitor_JLC:SW-SMD_4P-L5.1-W5.1-P3.70-LS6.5-TL_H1.5` | ⚠ Custom | Switch | 2 | [XKB TS-1187A](../references/datasheets/SWITCH_XKB_TS-1187A_Tactile_SMD_5.1x5.1_H1.5_C318884_Datasheet.pdf) |
| JST PH 3-pin right-angle | `Connector_JST:JST_PH_S3B-PH-K_1x03_P2.00mm_Horizontal` | KiCad std | Connector | 1 | [JST PH](../references/datasheets/CONN_JST_PH_2.0mm_Crimp_Connector_Family_Datasheet.pdf) |
| JST PH 2-pin right-angle | `Connector_JST:JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal` | KiCad std | Connector | 1 | [JST PH](../references/datasheets/CONN_JST_PH_2.0mm_Crimp_Connector_Family_Datasheet.pdf) |
| SMD test pad Ø1.5 mm | `TestPoint:TestPoint_Pad_D1.5mm` | KiCad std | Test | 9 | — |
| THT test point Ø2.0 / drill 1.0 mm | `TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm` | KiCad std | Test | 3 | — |
| M3 mounting hole Ø3.2 mm | `MountingHole:MountingHole_3.2mm_M3` | KiCad std | Mechanical | 4 | — |

## Placements by package

### MLCC 0603 (1608 metric)

`Capacitor_SMD:C_0603_1608Metric` · **Library:** KiCad std · **Sheets:** 02 USB-C, 03 3V3 Power, 04 ESP32-S3 Core, 05 I2C Sensors, 06 Soil ADC, 07 Charger · **Datasheet:** [Samsung MLCC](../references/datasheets/CAP_Samsung_MLCC_Catalogue_2015-11.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| C1 | 100nF | 02 USB-C | C14663 |
| C3 | 100nF | 02 USB-C | C14663 |
| C4 | 100nF | 02 USB-C | C14663 |
| C6 | 1uF | 03 3V3 Power | C15849 |
| C7 | 1uF | 03 3V3 Power | C15849 |
| C8 | 1uF | 04 ESP32-S3 Core | C15849 |
| C10 | 100nF | 04 ESP32-S3 Core | C14663 |
| C11 | 100nF | 05 I2C Sensors | C14663 |
| C12 | 100nF | 05 I2C Sensors | C14663 |
| C13 | 100nF | 05 I2C Sensors | C14663 |
| C14 | 100nF | 06 Soil ADC | C14663 |
| C15 | 4.7uF | 07 Charger | C19666 |
| C16 | 4.7uF | 07 Charger | C19666 |
| C17 | 100nF | 07 Charger | C14663 |
| C18 | 1uF | 07 Charger | C15849 |

### MLCC 0805 (2012 metric)

`Capacitor_SMD:C_0805_2012Metric` · **Library:** KiCad std · **Sheets:** 02 USB-C, 03 3V3 Power, 04 ESP32-S3 Core · **Datasheet:** [Samsung MLCC](../references/datasheets/CAP_Samsung_MLCC_Catalogue_2015-11.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| C2 | 10uF | 02 USB-C | C15850 |
| C5 | 10uF | 03 3V3 Power | C15850 |
| C9 | 22uF | 04 ESP32-S3 Core | C45783 |

### Chip resistor 0603 (1608 metric)

`Resistor_SMD:R_0603_1608Metric` · **Library:** KiCad std · **Sheets:** 02 USB-C, 03 3V3 Power, 04 ESP32-S3 Core, 05 I2C Sensors, 06 Soil ADC, 07 Charger

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| R1 | 22 | 02 USB-C | C23345 |
| R2 | 22 | 02 USB-C | C23345 |
| R3 | 5.1k | 02 USB-C | C23186 |
| R4 | 5.1k | 02 USB-C | C23186 |
| R5 | 10k | 03 3V3 Power | C25804 |
| R6 | 10k | 04 ESP32-S3 Core | C25804 |
| R7 | 10k | 04 ESP32-S3 Core | C25804 |
| R8 | 4.7k | 05 I2C Sensors | C23162 |
| R9 | 4.7k | 05 I2C Sensors | C23162 |
| R10 | 100k | 06 Soil ADC | C25803 |
| R11 | 100k | 06 Soil ADC | C25803 |
| R12 | 470 | 07 Charger | C23179 |
| R13 | 10k | 07 Charger | C25804 |
| R14 | 100k 1% | 07 Charger | C25803 |
| R15 | 100k 1% | 07 Charger | C25803 |
| R16 | 10k | 07 Charger | C25804 |

### Chip LED 0603

`LED_SMD:LED_0603_1608Metric` · **Library:** KiCad std · **Sheets:** 03 3V3 Power, 07 Charger · **Datasheet:** [XINGLIGHT XL-0603QYGC](../references/datasheets/LED_XINGLIGHT_XL-0603QYGC_YellowGreen_0603_C2289_Datasheet.pdf) · [KENTO KT-0603R](../references/datasheets/LED_KENTO_KT-0603R_Red_0603_C2286_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| D2 | LED_YG | 03 3V3 Power | C2289 |
| D3 | LED_RED | 07 Charger | C2286 |

### SMF5.0A TVS diode

`Diode_SMD:D_SOD-123F` · **Library:** KiCad std · **Sheets:** 02 USB-C · **Datasheet:** [MDD SMF5](../references/datasheets/TVS_MDD_SMF5_0A_5V_Unidirectional_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| D1 | SMF5.0A | 02 USB-C | C2980403 |

### SS14 Schottky 1A/40V

`Diode_SMD:D_SMA` · **Library:** KiCad std · **Sheets:** 07 Charger · **Datasheet:** [SS14 Schottky](../references/datasheets/DIODE_SS14_Schottky_1A40V_Family_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| D4 | SS14 | 07 Charger | C2480 |

### Resettable PPTC fuse 1206

`Fuse:Fuse_1206_3216Metric` · **Library:** KiCad std · **Sheets:** 02 USB-C · **Datasheet:** [Littelfuse 1206L](../references/datasheets/FUSE_Littelfuse_1206L_Resettable_PPTC_Datasheet_2024.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| F1 | 1206L075/16WR | 02 USB-C | C371166 |

### HRO USB-C receptacle, 16P

`Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` · **Library:** KiCad std · **Sheets:** 02 USB-C · **Datasheet:** [TYPE-C-31-M-12 Receptacle](../references/datasheets/USBC_TYPE-C-31-M-12_Receptacle_C165948_Footprint_Drawing.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| J1 | USB_C_Receptacle_USB2.0_16P | 02 USB-C | C165948 |

### SOT-23-6 — ESD protection

`Package_TO_SOT_SMD:SOT-23-6` · **Library:** KiCad std · **Sheets:** 02 USB-C · **Datasheet:** [ST USBLC6-2SC6](../references/datasheets/ESD_ST_USBLC6-2SC6_USB2_DataLine_Protection_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| U1 | USBLC6-2SC6 | 02 USB-C | C7519 |

### SOT-23-5 — LDO + LiPo charger

`Package_TO_SOT_SMD:SOT-23-5` · **Library:** KiCad std · **Sheets:** 03 3V3 Power, 07 Charger · **Datasheet:** [Diodes AP2112K-3.3](../references/datasheets/LDO_Diodes_AP2112K-3.3_600mA_3V3_Regulator_Datasheet.pdf) · [Microchip MCP73831](../references/datasheets/CHARGER_Microchip_MCP73831_LiPo_Linear_Charger_Datasheet_DS20001984H.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| U2 | AP2112K-3.3 | 03 3V3 Power | C51118 |
| U6 | MCP73831-2-OT | 07 Charger | C424093 |

### SOT-23 — P-channel MOSFET

`Package_TO_SOT_SMD:SOT-23` · **Library:** KiCad std · **Sheets:** 06 Soil ADC, 07 Charger · **Datasheet:** [AlphaOmega AO3401A](../references/datasheets/MOSFET_AlphaOmega_AO3401A_PchannelMOSFET_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| Q1 | AO3401A | 06 Soil ADC | C15127 |
| Q2 | AO3401A | 07 Charger | C15127 |
| Q3 | AO3401A | 07 Charger | C15127 |

### ESP32-S3-WROOM-1 module

`RF_Module:ESP32-S3-WROOM-1` · **Library:** KiCad std · **Sheets:** 04 ESP32-S3 Core · **Datasheet:** [Espressif WROOM-1](../references/datasheets/ESP32S3_Espressif_WROOM-1_WROOM-1U_Module_Datasheet_v1.8.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| U3 | ESP32-S3-WROOM-1 | 04 ESP32-S3 Core | C2913198 |

### Bosch LGA-8 — BME280

`Package_LGA:Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering` · **Library:** KiCad std · **Sheets:** 05 I2C Sensors · **Datasheet:** [Bosch BME280](../references/datasheets/SENSOR_Bosch_BME280_Temperature_Humidity_Pressure_Datasheet_Rev1.24_2024.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| U4 | BME280 | 05 I2C Sensors | C92489 |

### Vishay VEML7700-TT light sensor

`PlantMonitor_JLC:SENSOR-SMD_EML7700-TT` · **Library:** ⚠ Custom · **Sheets:** 05 I2C Sensors · **Datasheet:** [Vishay VEML7700](../references/datasheets/SENSOR_Vishay_VEML7700_Ambient_Light_I2C_Datasheet_Rev1.8_2024.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| U5 | VEML7700-TT | 05 I2C Sensors | C1850416 |

### SMD tactile switch 5.1 × 5.1 mm (XKB TS-1187A)

`PlantMonitor_JLC:SW-SMD_4P-L5.1-W5.1-P3.70-LS6.5-TL_H1.5` · **Library:** ⚠ Custom · **Sheets:** 04 ESP32-S3 Core · **Datasheet:** [XKB TS-1187A](../references/datasheets/SWITCH_XKB_TS-1187A_Tactile_SMD_5.1x5.1_H1.5_C318884_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| SW1 | SW_Push | 04 ESP32-S3 Core | C318884 |
| SW2 | SW_Push | 04 ESP32-S3 Core | C318884 |

### JST PH 3-pin right-angle

`Connector_JST:JST_PH_S3B-PH-K_1x03_P2.00mm_Horizontal` · **Library:** KiCad std · **Sheets:** 06 Soil ADC · **Datasheet:** [JST PH](../references/datasheets/CONN_JST_PH_2.0mm_Crimp_Connector_Family_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| J2 | Conn_01x03 | 06 Soil ADC | C157929 |

### JST PH 2-pin right-angle

`Connector_JST:JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal` · **Library:** KiCad std · **Sheets:** 07 Charger · **Datasheet:** [JST PH](../references/datasheets/CONN_JST_PH_2.0mm_Crimp_Connector_Family_Datasheet.pdf)

| Ref | Value | Sheet | JLC # |
|---|---|---|---|
| J3 | Conn_01x02 | 07 Charger | C173752 |

### SMD test pad Ø1.5 mm

`TestPoint:TestPoint_Pad_D1.5mm` · **Library:** KiCad std · **Sheets:** 04 ESP32-S3 Core, 08 Mechanical

| Ref | Value | Sheet |
|---|---|---|
| TP1 | TestPoint | 08 Mechanical |
| TP2 | TestPoint | 08 Mechanical |
| TP3 | TestPoint | 08 Mechanical |
| TP4 | TestPoint | 08 Mechanical |
| TP5 | TestPoint | 08 Mechanical |
| TP6 | TestPoint | 08 Mechanical |
| TP7 | TestPoint | 08 Mechanical |
| TP9 | TestPoint | 04 ESP32-S3 Core |
| TP10 | TestPoint | 04 ESP32-S3 Core |

### THT test point Ø2.0 / drill 1.0 mm

`TestPoint:TestPoint_THTPad_D2.0mm_Drill1.0mm` · **Library:** KiCad std · **Sheets:** 04 ESP32-S3 Core, 08 Mechanical

| Ref | Value | Sheet |
|---|---|---|
| TP8 | TestPoint | 08 Mechanical |
| TP11 | TestPoint | 04 ESP32-S3 Core |
| TP12 | TestPoint | 04 ESP32-S3 Core |

### M3 mounting hole Ø3.2 mm

`MountingHole:MountingHole_3.2mm_M3` · **Library:** KiCad std · **Sheets:** 08 Mechanical

| Ref | Value | Sheet |
|---|---|---|
| H1 | MountingHole | 08 Mechanical |
| H2 | MountingHole | 08 Mechanical |
| H3 | MountingHole | 08 Mechanical |
| H4 | MountingHole | 08 Mechanical |
