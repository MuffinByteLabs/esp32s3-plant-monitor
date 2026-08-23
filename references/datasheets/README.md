# Datasheet index

Every part on the board, mapped to the document it was designed against. Reference designators are
as-built and match the [schematic](../../docs/ESP32S3_PlantMonitor_RevA_Schematic.pdf) and the
[ordered BOM](../../fabrication/revA/BOM-ESP32S3_PlantMonitor.csv).

## On the board

| Ref | Part | JLC # | Datasheet |
|---|---|---|---|
| U1 | USBLC6-2SC6 USB ESD array | C7519 | [ESD_ST_USBLC6-2SC6](ESD_ST_USBLC6-2SC6_USB2_DataLine_Protection_Datasheet.pdf) |
| U2 | AP2112K-3.3 LDO | C51118 | [LDO_Diodes_AP2112K-3.3](LDO_Diodes_AP2112K-3.3_600mA_3V3_Regulator_Datasheet.pdf) |
| U3 | ESP32-S3-WROOM-1-N8 module | C2913198 | [ESP32S3_Espressif_WROOM-1](ESP32S3_Espressif_WROOM-1_WROOM-1U_Module_Datasheet_v1.8.pdf) |
| U4 | BME280 temp / humidity / pressure | C92489 | [SENSOR_Bosch_BME280](SENSOR_Bosch_BME280_Temperature_Humidity_Pressure_Datasheet_Rev1.24_2024.pdf) |
| U5 | VEML7700-TT ambient light | C1850416 | [SENSOR_Vishay_VEML7700](SENSOR_Vishay_VEML7700_Ambient_Light_I2C_Datasheet_Rev1.8_2024.pdf) |
| U6 | MCP73831 LiPo charger | C424093 | [CHARGER_Microchip_MCP73831](CHARGER_Microchip_MCP73831_LiPo_Linear_Charger_Datasheet_DS20001984H.pdf) |
| Q1–Q3 | AO3401A P-channel MOSFET | C15127 | [MOSFET_AlphaOmega_AO3401A](MOSFET_AlphaOmega_AO3401A_PchannelMOSFET_Datasheet.pdf) |
| D1 | SMF5.0A TVS | C2980403 | [TVS_MDD_SMF5_0A](TVS_MDD_SMF5_0A_5V_Unidirectional_Datasheet.pdf) |
| D2 | Yellow-green LED 0603 (power) | C2289 | [LED_XINGLIGHT_XL-0603QYGC](LED_XINGLIGHT_XL-0603QYGC_YellowGreen_0603_C2289_Datasheet.pdf) |
| D3 | Red LED 0603 (charge) | C2286 | [LED_KENTO_KT-0603R](LED_KENTO_KT-0603R_Red_0603_C2286_Datasheet.pdf) |
| D4 | SS14 Schottky 1 A / 40 V | C2480 | [DIODE_SS14](DIODE_SS14_Schottky_1A40V_Family_Datasheet.pdf) |
| F1 | 1206L075/16WR resettable PPTC fuse | C371166 | [FUSE_Littelfuse_1206L](FUSE_Littelfuse_1206L_Resettable_PPTC_Datasheet_2024.pdf) |
| J1 | TYPE-C-31-M-12 USB-C receptacle | C165948 | [USBC_TYPE-C-31-M-12](USBC_TYPE-C-31-M-12_Receptacle_C165948_Footprint_Drawing.pdf) |
| J2, J3 | JST PH 2.0 mm headers | C157929 / C173752 | [CONN_JST_PH_2.0mm](CONN_JST_PH_2.0mm_Crimp_Connector_Family_Datasheet.pdf) |
| SW1, SW2 | XKB TS-1187A tactile switch | C318884 | [SWITCH_XKB_TS-1187A](SWITCH_XKB_TS-1187A_Tactile_SMD_5.1x5.1_H1.5_C318884_Datasheet.pdf) |
| C1–C18 | Samsung MLCC 0603 / 0805 | various | [CAP_Samsung_MLCC_Catalogue](CAP_Samsung_MLCC_Catalogue_2015-11.pdf) — general catalogue, covers both C19666 (4.7 µF 0603) and C45783 (22 µF 0805) |
| R1–R16 | Chip resistors 0603 | various | Generic 0603 thick-film; values and JLC numbers in the [BOM](../../docs/ESP32S3_PlantMonitor_RevA_BOM.md) |

## Off-board — bought separately

| Part | Plugs into | Datasheet |
|---|---|---|
| DFRobot SEN0193 capacitive soil probe | J2 | [SENSOR_DFRobot_SEN0193](SENSOR_DFRobot_SEN0193_Capacitive_Soil_Moisture_Module_Web_Datasheet.pdf) |
| PKCell LP503035 500 mAh 1S LiPo with PCM | J3 | [BATTERY_C1854_PKCell](BATTERY_C1854_PKCell_Datasheet_Li-Polymer_503035_500mAh_3.7V_with_PCM.pdf) |

## Design guidance

| Document | Used for |
|---|---|
| [ESP32-S3 Hardware Design Guidelines](ESP32S3_Espressif_Hardware_Design_Guidelines_2026-06-23.pdf) | Power entrance, decoupling, strapping pins, antenna keep-out, ADC handling — the rules in `docs/Hard_Rules_Layout_RevA.md` trace back here |
| [DevKitC-1 reference schematic](../reference-designs/ESP32S3_Espressif_DevKitC-1_Schematic_v1.1_2022-04-13.pdf) | Reference implementation cross-check |

---

All datasheets remain the property of their respective manufacturers and are included here for
convenience. Copyrighted standards are **not** redistributed — see [`../standards/`](../standards/).
