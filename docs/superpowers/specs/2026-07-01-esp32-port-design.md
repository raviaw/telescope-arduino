# ESP32 Port — Telescope Mount Firmware Design

**Date:** 2026-07-01
**Target repo:** `telescope-arduino` (firmware in `main_sketch/`)
**Status:** Design — approved sections, pending final spec review

## 0. Revision — elevated scope (major rework approved)

Scope was raised from a minimal-change port to a proper rework (rewiring accepted). Where the
sections below differ, **these decisions win:**

- **Firmware = modular re-architecture** (not a 1:1 `.ino` port). Target structure:

  ```
  src/
    hal/     Display, Buttons(MCP), Analog(ADS), MotorAxis(step+PCNT+enable), Clock
    comms/   BluetoothLink, JsonProtocol
    astro/   Ephemeris wrap, Calibration, Backslash   (proven logic kept)
    ui/      MenuScreens (OLED#2), SkyMap (OLED#1)
    storage/ Nvs      state/ State object (replaces global soup)
  main.cpp   setup()/loop() wire modules together
  ```

- **TB6600 rewired common-cathode** — active-high STEP/DIR at 3.3 V (see §4.1).
- **Driver ENABLE wired** per-axis (GPIO16/17) for idle auto-disable.
- **2× MCP23017** (0x20 inputs, 0x21 outputs) — headroom instead of a maxed single chip.
- **NVS calibration persistence is core scope** (no longer Phase 2).
- **DS3231 RTC kept** (I²C 0x68, no extra pins) as the primary time source; phone/NTP sync it.
- **Bluetooth telemetry fully implemented** (not just re-enabled), with clean JSON framing.

## 1. Goal

Replace the current three-microcontroller mount controller (Arduino Mega 2560 + two
encoder-reading Arduino Nanos) plus the HC-05 Bluetooth module and DS3231 RTC with a
**single classic ESP32 (ESP-WROOM-32, 30-pin DOIT DevKit V1)**.

The `GreatGrandeurs` Android app must keep working **without changes** — the Bluetooth
JSON protocol (command set + telemetry keys) is preserved exactly.

## 2. Why the classic ESP32 fits

- **Bluetooth Classic / SPP** on-chip → the app's RFCOMM transport works unchanged
  (an S3/C3 would be BLE-only and force an app rewrite).
- **PCNT** peripheral (8 hardware quadrature counters) → reads both axis encoders and the
  menu knob directly, absorbing both Nanos and deleting the custom serial protocol.
- **WiFi** on-chip → optional NTP time source; enables reviving the dormant
  `telescope-server` over TCP later.
- **True 64-bit `double`** → fixes the AVR single-precision limitation the project README
  lists as future work.

## 3. Current vs. target architecture

| Concern | Current (Mega) | Target (ESP32) |
|---|---|---|
| Main MCU | Arduino Mega 2560 | ESP-WROOM-32 |
| Axis encoders | 2× Nano, each streams 8-byte packets @57600 over UART | Read directly via PCNT (`ESP32Encoder`) |
| Bluetooth | HC-05 on `Serial1` @9600 | On-chip `BluetoothSerial` (SPP) |
| Time | DS3231 RTC on I²C | DS3231 kept (0x68); phone `time` command / NTP sync it |
| UARTs used | 4 (USB, BT, 2 encoders) | 1 (USB debug) |
| Human I/O | Native pins (analog ladder keypad, pots, joysticks, LEDs, 16×2 LCD) | I²C expanders (ADS1115 + MCP23017); 2× OLED |

## 4. Hardware design

### 4.1 Motors & drivers
- Motors: **NEMA 17 bipolar steppers** (confirmed by photo), coaxially coupled to the
  YT3806 encoders; mechanical reduction per existing build.
- Drivers: **TB6600** (opto-isolated `PUL±`, `DIR±`, `ENA±`; `VCC` 9–42 V; motor on `A±`/`B±`).
- **Opto wiring (revised — rewire to common-cathode):** tie all `–` pins (`PUL-`, `DIR-`, `ENA-`)
  to GND and drive the `+` pins from the ESP32. This gives **active-high `STEP`/`DIR` driven
  directly at 3.3 V** — no 5 V common, no `DIR` inversion. At ≤7 kHz step rates 3.3 V opto drive is
  comfortable (74HCT125 buffer only as a fallback).
- **`ENA` is now wired** per-axis (GPIO16/17) so `FastAccelStepper` auto-enable de-energizes idle
  motors.
- Keep the 6 DIP switches (microstep + current) **unchanged** so the existing speed constants
  (`MAX_HORIZONTAL_SPEED 7000`, etc.) stay valid.

### 4.2 Encoders — YT3806-1000PPR-NPN
- 1000 PPR quadrature (A/B), no index channel → incremental; absolute reference comes from
  star calibration (unchanged behavior).
- Powered 8–24 VDC. Output is **NPN open-collector**, so signal high level is set by an
  external pull-up — **pull A/B to 3.3 V** and the ESP32 reads clean 0↔3.3 V.
- The chosen PCNT pins (GPIO 34/35/36/39) are input-only with **no internal pull-ups**, so
  external pull-ups are required regardless — a natural fit.

**Per-encoder wiring (×2):**

| Encoder wire | Connects to |
|---|---|
| Red (V+) | +12 V from PSU |
| Black (0 V COMMON) | **GND, shared with ESP32 GND** |
| Green (OUT A) | 4.7 kΩ pull-up to **3.3 V**, then PCNT pin |
| White (OUT B) | 4.7 kΩ pull-up to **3.3 V**, then PCNT pin |
| Shield | GND |

> **Hard rule:** pull A/B to 3.3 V, **never** to the 12 V encoder supply — that would
> destroy the ESP32 pin.

Resolution stays consistent: 1000 PPR × 4 (quadrature) = 4000 counts/rev, matching the
Nano's `Encoder.read()` behavior.

### 4.3 I²C bus (SDA = GPIO21, SCL = GPIO22; pull-ups to 3.3 V)

| Device | Addr | Purpose |
|---|---|---|
| SSD1306 OLED #1 | 0x3C | Sky-map graphical view (existing `refreshOled`) |
| SSD1306 OLED #2 | 0x3D | Text menu/status (ported from LCD) |
| MPU6050 IMU | 0x69 | Accelerometer (kept) |
| ADS1115 | 0x48 | 4 analog: horiz pot, vert pot, left-joystick, right-joystick |
| MCP23017 #1 | 0x20 | inputs: keypad + control buttons |
| MCP23017 #2 | 0x21 | outputs: status LEDs + laser + spare |
| DS3231 RTC | 0x68 | timekeeping (kept — no extra pins) |

The 2nd OLED has an on-board address jumper (`0x78` / `0x7A` = 0x3C / 0x3D 7-bit); set it to
**0x3D**, so both OLEDs share the one I²C bus — no second bus, no extra chip. The DS3231 RTC
stays on the bus at 0x68 (no extra pins).

### 4.4 Native ESP32 GPIO (timing-critical only)

| Function | GPIO | Notes |
|---|---|---|
| Stepper V — STEP / DIR | 25 / 26 | to TB6600 |
| Stepper H — STEP / DIR | 32 / 33 | to TB6600 |
| Encoder V — A / B (PCNT) | 34 / 35 | input-only, external pull-ups |
| Encoder H — A / B (PCNT) | 36 / 39 | input-only (VP/VN), external pull-ups |
| Menu knob — A / B (PCNT) | 27 / 14 | rotary needs hardware counting |
| I²C SDA / SCL | 21 / 22 | main bus (see 4.3) |
| Breathing LED (PWM) | 13 | `ledc` (MCP23017 can't PWM) |
| USB debug serial | TX0/RX0 | flashing + logs |
| ENA V / ENA H | 16 / 17 | driver enable (auto-disable when idle) |

~14–16 native pins of ~23 usable — comfortable headroom.

### 4.5 Offload chip loads
- **ADS1115 (4ch):** the 4 motion analog inputs only (`HORIZONTAL_POT`, `VERTICAL_POT`,
  `HORIZONTAL_JOYSTICK_LEFT`, `VERTICAL_JOYSTICK_RIGHT`). A software adapter scales the
  16-bit reading into the 0–1023 range the existing `translatePotValueToSpeed()` /
  512-center logic expects, leaving that math untouched. Unused `SPEED_POT` stays dropped.
- **2× MCP23017 (revised):** inputs on **0x20** (5 keypad buttons replacing the analog ladder +
  5 control buttons: action, knob-push, L-joystick, R-joystick, enable-pot), outputs on **0x21**
  (4 status LEDs + laser + spare). Two chips give comfortable headroom instead of a maxed one.
- Breathing LED stays native (GPIO13, `ledc`) because it needs PWM.

### 4.6 Power tree
Reuses the existing supplies and regulators:

| Rail | Source | Feeds |
|---|---|---|
| 24 V / 15 A | Cheng Liang P360W24V | TB6600 `VCC` (motor power) |
| 12 V / 20 A | Durawell S-240-12 | encoder `V+` (8–24 V), and the 5 V reg |
| 5 V | LM1117T-5.0 (existing) | ESP32 `VIN` |
| 3.3 V | AMS1117-3.3 (existing, from 5 V) | all I²C devices, I²C + encoder pull-ups, **TB6600 opto common** |

Rules:
- Power the I²C devices (OLED×2, MPU6050, ADS1115, MCP23017) from **3.3 V** so the bus idles at
  3.3 V — powering them at 5 V would put 5 V on the ESP32 SDA/SCL and damage it.
- Feed the ESP32 via `VIN` (5 V); its on-board regulator makes the chip's 3.3 V. Do not back-feed
  the `3V3` pin. Peripherals use the separate AMS1117-3.3 rail (common ground).
- Common ground across ESP32, encoders, drivers, and both PSUs is mandatory.

## 5. Firmware design

### 5.1 Build system / libraries (`platformio.ini`)
- Board → `esp32dev`, platform `espressif32`, framework `arduino`.
- **Remove:** `LiquidCrystal` (LCD dropped), `Encoder` (AVR), `SoftwareSerial`, `SPI` (unused).
- **Add:** `ESP32Encoder` (PCNT), `Adafruit_ADS1X15`, an MCP23017 library;
  `BluetoothSerial` is built into the ESP32 core.
- **Keep:** `RTClib` (DS3231 kept), `ArduinoJson`, `elapsedMillis`, `Adafruit_GFX`,
  `Adafruit_SSD1306`, `Ephemeris`.

### 5.2 File-by-file change map

| File | Change |
|---|---|
| `platformio.ini` | Board + library changes above. |
| `MotorWithEncoder.*` | Biggest change. Drop `HardwareSerial*`, `updateEncoderFromSerial()`, the 8-byte bit-decoder, `_encoderBuffer`/`_discardBuffer`. Add an `ESP32Encoder` member; `readEncoderPosition()` → `getCount()`. **Keep** the direction-tracking + backlash state machine, fed from the new count. |
| encoder Nano sketches | Deleted (both). |
| `other_android.ino` | `Serial1`/HC-05 → `BluetoothSerial`. Re-enable the currently-commented `bluetoothSerialAvailable()` (RX) and `reportBluetooth()` (telemetry). JSON command set + telemetry keys unchanged. |
| `other_time.ino` | Keep `rtc.now()` (DS3231). The phone `time` command still `rtc.adjust`s it (`parseReceivedTimeString`); optional NTP sync over WiFi. |
| `other_lcd.ino` | Reimplement `printLcdAt` / `printLcdNumber` / `printLcdFloatingPointNumber` / `renderMenuOptions` against Adafruit_GFX on **OLED #2**. `registerButton()` reads MCP23017 instead of the analog ladder + `digitalRead`. Menu flow/logic unchanged. |
| `other_oled.ino` | Stays — OLED #1 (sky map). |
| `Globals.hpp` | Full pin-map rewrite to the GPIO table in 4.4; remove LCD/serial-encoder globals (RTC kept). |
| `main_sketch.ino` | `setup()`: init `ESP32Encoder` ×3 (2 axes + knob), ADS1115, 2× MCP23017, DS3231 RTC, `BluetoothSerial`, 2× SSD1306; remove `Serial1/2/3` init. Loop timer structure unchanged. |

### 5.3 Encoder subsystem
PCNT replaces the serial link. Higher resolution, no serial latency, no packet-loss
filtering, fewer parts. Rejected alternative: keeping the Nanos on one ESP32 UART — strictly
worse. `ESP32Encoder` full-quad matches the old 4× decoding; the knob's `read()/2`
half-stepping scaling is preserved.

### 5.4 Bluetooth transport
`BluetoothSerial SerialBT; SerialBT.begin("Telescope");`. RX path deserializes JSON from
`SerialBT`; TX path (`reportBluetooth`) writes telemetry to `SerialBT`. Both are currently
commented out in `loop()` and get re-enabled as part of the port. Protocol bytes are
identical, so `GreatGrandeurs` pairs and talks to the ESP32 as it did to the HC-05.

### 5.5 Time source
Primary source is the **DS3231 RTC** (kept, 0x68), read in `calculateTime()` as today. The phone
`time` command still syncs it (`rtc.adjust`), and NTP-over-WiFi is an optional extra sync. This
keeps correct time at power-on with no phone or network.

### 5.6 Human interface
- **Analog:** ADS1115 → scaled to 0–1023 → fed into existing pot/joystick logic unchanged.
- **Digital:** MCP23017 for all buttons and LEDs; `registerButton()` reads discrete pins
  (keypad is now 5 discrete buttons, not a resistor ladder). `digitalWrite` for LEDs/laser
  become `mcp.digitalWrite`.
- **Knob:** `ESP32Encoder` on native PCNT pins; `knob.read()/2` behavior preserved.

### 5.7 Displays
- OLED #1: existing `refreshOled()` sky map.
- OLED #2: ported text menu/status. A 128×64 panel at text size 1 gives ~21×8 chars —
  more than the old 16×2, so the layout can stay or be improved.

## 6. What stays identical
Astronomy math (`calculateEverything`, Ephemeris), the calibration flow, the
backlash-compensation algorithm, the star catalog, and the **entire Bluetooth JSON
protocol**. No Android app changes.

## 7. Optional / Phase 2 (not core scope)
- **NVS calibration persistence:** re-enable the currently-stubbed EEPROM save/load using
  ESP32 `Preferences` so calibration survives power-off. Recommended but deferrable.
- **WiFi/NTP time** and **`telescope-server` over TCP**: enabled by the hardware, not built
  in this port.

## 8. Open confirmations (do not block architecture)
1. ~~TB6600 wiring~~ — **resolved:** TB6600; rewire **common-cathode** (`–` to GND, drive `+`); `ENA` wired per-axis; keep DIP switches. Record the 6 DIP positions when convenient.
2. ~~OLED addresses~~ — **resolved:** 2nd OLED set to 0x3D via its `0x78`/`0x7A` jumper.
3. ~~Power rails~~ — **resolved:** 24 V (motors) + 12 V (encoders); existing LM1117-5.0 → 5 V (ESP32 `VIN`) and AMS1117-3.3 → 3.3 V (I²C, pull-ups, opto common).

## 9. Risks / notes
- With 2× MCP23017 (0x20 inputs / 0x21 outputs) there is ample digital headroom.
- ESP32 ADC2 pins are unusable while WiFi is active; this design keeps all analog on the
  ADS1115, so it is unaffected.
- BluetoothSerial + WiFi both use the radio; if NTP is added, verify coexistence or fetch
  time only at connect.
- Porting the LCD menu to OLED is the largest single code task; it is well-contained behind
  the existing `printLcd*` / `renderMenuOptions` helpers.
