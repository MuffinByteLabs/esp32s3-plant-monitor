# Firmware — the hardware contract

No firmware is written yet; that is Rev B work. This file specifies what the *hardware* requires of it, so the board's safety and measurement behaviour is on record before any code exists. Target: ESP-IDF or PlatformIO. Pin assignments are in [`../docs/PinMap_CheatSheet.md`](../docs/PinMap_CheatSheet.md).

| Duty | Rule | Why, and where it comes from |
|---|---|---|
| **Wi-Fi TX cutoff** | No transmit below **≈3.5 V** battery | A regulator rule, not a battery rule: the AP2112K needs ≈200 mV of headroom to hold 3.3 V through a 355 mA TX peak — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §4 |
| **Deep-sleep cutoff** | Shut everything down at **3.0 V** | The pack's protection module does not trip until 2.5 V. That half-volt gap belongs to firmware and nothing else — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §3 |
| **Battery reporting** | Gate battery percentage on **USB absent** | With USB connected and no cell fitted, the charger floats VBAT to ≈4.2 V and BAT_SENSE reads "full" |
| **Soil measurement** | GPIO21 LOW → wait ≥200 ms → read GPIO1 → GPIO21 HIGH | The probe is firmware-switched so it does not drain the cell between readings |
| **Soil calibration** | Dry ≈ **2900** raw, wet ≈ **1465** raw (12-bit, ATTEN=3) | Breadboard values through the as-built input chain. Re-record on the PCB at bring-up step D-11 before trusting absolute readings — [`Engineering_Notes.md`](../docs/Engineering_Notes.md) §1 |
| **I²C** | BME280 at **0x76**, VEML7700 at **0x10**; `Wire.begin(38, 39)` — SDA first | Bus re-pinned to IO38/IO39 during layout — [`PinMap_CheatSheet.md`](../docs/PinMap_CheatSheet.md) |
| **Brown-out** | Expect a ≈50–100 ms dip on VSYS at USB unplug | Discrete load-sharing hand-over. If a reset appears there at low battery, relax the brown-out detector across the transition — [`BringUp_Guide.md`](../docs/BringUp_Guide.md) step 9 |
