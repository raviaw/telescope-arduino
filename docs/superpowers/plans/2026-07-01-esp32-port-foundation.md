# ESP32 Port — Plan 1: Foundation & Core Logic

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stand up the ESP32 PlatformIO project with a host-side unit-test harness, and build the hardware-independent core logic (math, calibration mapping, backlash decisions, Bluetooth JSON protocol) as tested libraries — all runnable on a PC with no hardware.

**Architecture:** Pure logic lives in PlatformIO `lib/` libraries with **no `Arduino.h` dependency**, so it compiles and unit-tests under the `native` environment (Unity) as well as on the ESP32. Hardware code (added in later plans) lives under `src/` and consumes these libraries. This is the first slice of the modular re-architecture in `docs/superpowers/specs/2026-07-01-esp32-port-design.md`.

**Tech Stack:** ESP32 (`esp32dev`), Arduino framework, C++17, ArduinoJson 7, PlatformIO + Unity for native tests.

## Global Constraints

- Target board: `esp32dev` (classic ESP-WROOM-32). Native test env: `native`.
- Language standard: **C++17** (`-std=gnu++17`).
- ArduinoJson **v7** API (`JsonDocument`, not the deprecated `StaticJsonDocument`).
- **Bluetooth protocol is a fixed contract with the Android app** — command strings and telemetry keys must match `GreatGrandeurs` exactly. Commands: `time`, `erase`, `master`, `slave`, `lights-on`, `lights-off`, `laser-on`, `laser-off`, `c-start`, `c-move`, `c-stop`, `c-save`, `c-done`, `find-star`, `go`, `move-start`, `move`, `move-done`, `menu-main`, `measure-backslash`. Telemetry keys: `ra`, `dec`, `last-star-ra`, `last-star-dec`, `azm`, `alt`, `c-m-azm`, `c-m-alt`, `calibrated`, `activeMode`, `hor-motor`, `ver-motor`, `hor-encoder`, `ver-encoder`, `acc-x`, `acc-y`, `acc-z`.
- Encoder resolution assumption preserved: quadrature counts are opaque `long`s; logic must not hardcode counts-per-rev.
- Pure logic libraries under `lib/` MUST NOT include `<Arduino.h>` or any hardware header.
- Work on branch `esp32-port`. Commit after every task.

> **Pre-req (do first, outside these tasks):** the working tree has pre-existing uncommitted edits to `main_sketch/.../*.ino`, `Globals.hpp`, `MotorWithEncoder.*`, and `encoder.ino`. Commit or stash those before starting so the rewrite doesn't collide with or bury them. Confirm with the maintainer which to keep.

---

### Task 1: Dual-environment PlatformIO scaffold + smoke test

Stand up the `native` test env alongside `esp32dev` and prove the Unity harness runs on the host.

**Files:**
- Create: `main_sketch/platformio.ini` (replace existing)
- Create: `lib/core/Maths.h` (placeholder header so LDF has a lib to find)
- Test: `main_sketch/test/test_smoke/test_smoke.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: a working `pio test -e native` command; the `lib/` layout other tasks add to.

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_smoke/test_smoke.cpp`:
```cpp
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_harness_runs() {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_harness_runs);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_smoke`
Expected: FAIL — build error, `native` environment not defined yet.

- [ ] **Step 3: Write the platformio.ini**

`main_sketch/platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_flags = -std=gnu++17
build_unflags = -std=gnu++11
lib_deps =
	bblanchon/ArduinoJson@^7.1.0
	madhephaestus/ESP32Encoder@^0.11.7
	adafruit/Adafruit ADS1X15@^2.5.0
	adafruit/Adafruit MCP23017 Arduino Library@^2.3.2
	adafruit/RTClib@^2.1.4
	adafruit/Adafruit GFX Library@^1.11.10
	adafruit/Adafruit SSD1306@^2.5.11
	marscaper/Ephemeris@^1.0.1
	pfeerick/elapsedMillis@^1.0.6

[env:native]
platform = native
test_framework = unity
build_flags = -std=gnu++17
lib_deps =
	bblanchon/ArduinoJson@^7.1.0
```

- [ ] **Step 4: Create the placeholder lib header**

`lib/core/Maths.h`:
```cpp
#pragma once
// Pure math helpers — no Arduino dependency. Filled in Task 2.
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_smoke`
Expected: PASS — `test_harness_runs`.

- [ ] **Step 6: Commit**

```bash
git add main_sketch/platformio.ini main_sketch/lib/core/Maths.h main_sketch/test/test_smoke/test_smoke.cpp
git commit -m "build: add native test env + esp32dev target scaffold"
```

---

### Task 2: `core` math helpers (`mapDouble`, `clamp`)

Port the two math primitives the whole system relies on, host-tested.

**Files:**
- Modify: `main_sketch/lib/core/Maths.h`
- Create: `main_sketch/lib/core/Maths.cpp`
- Test: `main_sketch/test/test_core/test_maths.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `double mapDouble(double x, double inMin, double inMax, double outMin, double outMax);`
  - `double clampDouble(double v, double lo, double hi);`
  - `int clampInt(int v, int lo, int hi);`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_core/test_maths.cpp`:
```cpp
#include <unity.h>
#include "Maths.h"

void setUp() {}
void tearDown() {}

void test_mapDouble_midpoint() {
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, mapDouble(0.5, 0.0, 1.0, 0.0, 10.0));
}
void test_mapDouble_inverted_range() {
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 90.0, mapDouble(0.0, 0.0, 100.0, 90.0, -90.0));
}
void test_clampDouble_bounds() {
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, clampDouble(9.0, -5.0, 5.0));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, -5.0, clampDouble(-9.0, -5.0, 5.0));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.0, clampDouble(1.0, -5.0, 5.0));
}
void test_clampInt_bounds() {
    TEST_ASSERT_EQUAL_INT(100, clampInt(250, -100, 100));
    TEST_ASSERT_EQUAL_INT(-100, clampInt(-250, -100, 100));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_mapDouble_midpoint);
    RUN_TEST(test_mapDouble_inverted_range);
    RUN_TEST(test_clampDouble_bounds);
    RUN_TEST(test_clampInt_bounds);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_core`
Expected: FAIL — `mapDouble`/`clampDouble`/`clampInt` undefined.

- [ ] **Step 3: Write the implementation**

`main_sketch/lib/core/Maths.h`:
```cpp
#pragma once

double mapDouble(double x, double inMin, double inMax, double outMin, double outMax);
double clampDouble(double v, double lo, double hi);
int clampInt(int v, int lo, int hi);
```

`main_sketch/lib/core/Maths.cpp`:
```cpp
#include "Maths.h"

double mapDouble(double x, double inMin, double inMax, double outMin, double outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

double clampDouble(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_core`
Expected: PASS — 4 tests.

- [ ] **Step 5: Commit**

```bash
git add main_sketch/lib/core/Maths.h main_sketch/lib/core/Maths.cpp main_sketch/test/test_core/test_maths.cpp
git commit -m "feat: add core math helpers (mapDouble, clamp) with native tests"
```

---

### Task 3: `core` joystick/pot speed mapping

Port `translatePotValueToSpeed` (0–1023 pot reading → −100..+100 speed) exactly, preserving the legacy dead-zone and integer-division behavior so motion feel is unchanged.

**Files:**
- Create: `main_sketch/lib/core/Speed.h`
- Create: `main_sketch/lib/core/Speed.cpp`
- Test: `main_sketch/test/test_core/test_speed.cpp`

**Interfaces:**
- Consumes: `clampInt` from `Maths.h`.
- Produces: `int translatePotValueToSpeed(int value, int multiplyFactor);`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_core/test_speed.cpp`:
```cpp
#include <unity.h>
#include "Speed.h"

void setUp() {}
void tearDown() {}

void test_center_deadzone_returns_zero() {
    TEST_ASSERT_EQUAL_INT(0, translatePotValueToSpeed(512, 1));
    TEST_ASSERT_EQUAL_INT(0, translatePotValueToSpeed(530, 1)); // within 30 of center
}
void test_full_positive() {
    // value 1023: testValue=511, multiply=+1, (511-30)/4 = 120 -> clamped to 100
    TEST_ASSERT_EQUAL_INT(100, translatePotValueToSpeed(1023, 1));
}
void test_full_negative() {
    // value 0: testValue=512, multiply=-1, (512-30)/4=120 -> clamp -> -100
    TEST_ASSERT_EQUAL_INT(-100, translatePotValueToSpeed(0, 1));
}
void test_multiply_factor_inverts() {
    TEST_ASSERT_EQUAL_INT(100, translatePotValueToSpeed(0, -1));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_center_deadzone_returns_zero);
    RUN_TEST(test_full_positive);
    RUN_TEST(test_full_negative);
    RUN_TEST(test_multiply_factor_inverts);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_core`
Expected: FAIL — `translatePotValueToSpeed` undefined.

- [ ] **Step 3: Write the implementation**

`main_sketch/lib/core/Speed.h`:
```cpp
#pragma once

// Maps a 0..1023 pot reading centered at 512 into a -100..+100 speed,
// with a +/-30 dead-zone. Preserves the original integer-division curve.
int translatePotValueToSpeed(int value, int multiplyFactor);
```

`main_sketch/lib/core/Speed.cpp`:
```cpp
#include "Speed.h"
#include "Maths.h"

int translatePotValueToSpeed(int value, int multiplyFactor) {
    int testValue = (value < 512) ? (512 - value) : (value - 512);
    int multiply = (value > 512) ? 1 : -1;
    if (testValue < 30) {
        return 0;
    }
    // NOTE: (480 / 100) is integer division == 4 in the original firmware; kept verbatim.
    return clampInt(multiplyFactor * multiply * ((testValue - 30) / (480 / 100)), -100, 100);
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_core`
Expected: PASS — all `test_core` tests (maths + speed).

- [ ] **Step 5: Commit**

```bash
git add main_sketch/lib/core/Speed.h main_sketch/lib/core/Speed.cpp main_sketch/test/test_core/test_speed.cpp
git commit -m "feat: add pot->speed mapping preserving legacy curve"
```

---

### Task 4: `astro` two-point calibration mapping

Extract the calibration math: two reference points, each pairing a sky coordinate with an encoder count and a motor position. Provides forward (sky→encoder target) and reverse (motor/encoder→sky) linear maps. Pure — no Ephemeris, no hardware.

**Files:**
- Create: `main_sketch/lib/astro/TwoPointCalibration.h`
- Create: `main_sketch/lib/astro/TwoPointCalibration.cpp`
- Test: `main_sketch/test/test_astro/test_calibration.cpp`

**Interfaces:**
- Consumes: `mapDouble` from `Maths.h`.
- Produces:
  - `struct CalPoint { double sky; long encoder; long motor; };`
  - `class TwoPointCalibration` with:
    - `void set(const CalPoint& a, const CalPoint& b);`
    - `bool calibrated() const;`
    - `double skyFromMotor(long motorPos) const;`
    - `double skyFromEncoder(long encoderPos) const;`
    - `long encoderFromSky(double sky) const;`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_astro/test_calibration.cpp`:
```cpp
#include <unity.h>
#include "TwoPointCalibration.h"

void setUp() {}
void tearDown() {}

void test_not_calibrated_by_default() {
    TwoPointCalibration c;
    TEST_ASSERT_FALSE(c.calibrated());
}

void test_sky_from_motor_linear() {
    TwoPointCalibration c;
    // point A: sky 10 deg @ motor 0 ; point B: sky 20 deg @ motor 1000
    c.set(CalPoint{10.0, 0, 0}, CalPoint{20.0, 1000, 1000});
    TEST_ASSERT_TRUE(c.calibrated());
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 15.0, c.skyFromMotor(500));
}

void test_encoder_from_sky_linear() {
    TwoPointCalibration c;
    c.set(CalPoint{10.0, 0, 0}, CalPoint{20.0, 4000, 1000});
    TEST_ASSERT_EQUAL_INT64(2000, c.encoderFromSky(15.0));
}

void test_sky_from_encoder_linear() {
    TwoPointCalibration c;
    c.set(CalPoint{10.0, 0, 0}, CalPoint{20.0, 4000, 1000});
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 12.5, c.skyFromEncoder(1000));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_not_calibrated_by_default);
    RUN_TEST(test_sky_from_motor_linear);
    RUN_TEST(test_encoder_from_sky_linear);
    RUN_TEST(test_sky_from_encoder_linear);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_astro`
Expected: FAIL — `TwoPointCalibration` undefined.

- [ ] **Step 3: Write the implementation**

`main_sketch/lib/astro/TwoPointCalibration.h`:
```cpp
#pragma once

struct CalPoint {
    double sky;    // sky coordinate (deg or hours, caller's unit)
    long encoder;  // quadrature count at that point
    long motor;    // stepper position at that point
};

class TwoPointCalibration {
  public:
    void set(const CalPoint& a, const CalPoint& b);
    bool calibrated() const { return calibrated_; }

    double skyFromMotor(long motorPos) const;
    double skyFromEncoder(long encoderPos) const;
    long encoderFromSky(double sky) const;

  private:
    CalPoint a_{0, 0, 0};
    CalPoint b_{0, 0, 0};
    bool calibrated_ = false;
};
```

`main_sketch/lib/astro/TwoPointCalibration.cpp`:
```cpp
#include "TwoPointCalibration.h"
#include "Maths.h"

void TwoPointCalibration::set(const CalPoint& a, const CalPoint& b) {
    a_ = a;
    b_ = b;
    calibrated_ = true;
}

double TwoPointCalibration::skyFromMotor(long motorPos) const {
    return mapDouble(motorPos, a_.motor, b_.motor, a_.sky, b_.sky);
}

double TwoPointCalibration::skyFromEncoder(long encoderPos) const {
    return mapDouble(encoderPos, a_.encoder, b_.encoder, a_.sky, b_.sky);
}

long TwoPointCalibration::encoderFromSky(double sky) const {
    return (long)mapDouble(sky, a_.sky, b_.sky, a_.encoder, b_.encoder);
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_astro`
Expected: PASS — 4 tests.

- [ ] **Step 5: Commit**

```bash
git add main_sketch/lib/astro/ main_sketch/test/test_astro/test_calibration.cpp
git commit -m "feat: add two-point calibration mapping with native tests"
```

---

### Task 5: `motion` backlash decision logic

Extract the pure decision logic from `MotorWithEncoder`: given a max-backlash value it computes speed thresholds, and given a distance it picks a speed level. No motor calls — those stay in the HAL (Plan 2).

**Files:**
- Create: `main_sketch/lib/motion/BacklashCompensator.h`
- Create: `main_sketch/lib/motion/BacklashCompensator.cpp`
- Test: `main_sketch/test/test_motion/test_backlash.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `struct SpeedLevels { int s1; int s2; int s3; int s4; int s5; };`
  - `class BacklashCompensator` with:
    - `BacklashCompensator(long maxBacklash, SpeedLevels levels);`
    - `int speedForForwardDistance(long absDiff) const;`
    - `int speedForBacklashRemaining(long remaining) const;`
    - `long maxBacklash() const;`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_motion/test_backlash.cpp`:
```cpp
#include <unity.h>
#include "BacklashCompensator.h"

void setUp() {}
void tearDown() {}

static SpeedLevels levels() { return SpeedLevels{10, 50, 1750, 3500, 7000}; }

void test_forward_distance_speed_buckets() {
    BacklashCompensator b(41000, levels());
    TEST_ASSERT_EQUAL_INT(7000, b.speedForForwardDistance(600)); // >500
    TEST_ASSERT_EQUAL_INT(3500, b.speedForForwardDistance(300)); // >200
    TEST_ASSERT_EQUAL_INT(1750, b.speedForForwardDistance(150)); // >100
    TEST_ASSERT_EQUAL_INT(50,   b.speedForForwardDistance(40));  // >30
    TEST_ASSERT_EQUAL_INT(10,   b.speedForForwardDistance(5));   // else
}

void test_backlash_remaining_uses_large_thresholds() {
    // maxBacklash 41000 (>30000): thresholds 500/100/50/20
    BacklashCompensator b(41000, levels());
    TEST_ASSERT_EQUAL_INT(7000, b.speedForBacklashRemaining(600)); // >500
    TEST_ASSERT_EQUAL_INT(3500, b.speedForBacklashRemaining(150)); // >100
    TEST_ASSERT_EQUAL_INT(1750, b.speedForBacklashRemaining(60));  // >50
    TEST_ASSERT_EQUAL_INT(50,   b.speedForBacklashRemaining(25));  // >20
    TEST_ASSERT_EQUAL_INT(10,   b.speedForBacklashRemaining(5));   // else
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_forward_distance_speed_buckets);
    RUN_TEST(test_backlash_remaining_uses_large_thresholds);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_motion`
Expected: FAIL — `BacklashCompensator` undefined.

- [ ] **Step 3: Write the implementation**

`main_sketch/lib/motion/BacklashCompensator.h`:
```cpp
#pragma once

struct SpeedLevels {
    int s1;  // crawl
    int s2;
    int s3;
    int s4;
    int s5;  // max
};

class BacklashCompensator {
  public:
    BacklashCompensator(long maxBacklash, SpeedLevels levels);

    // Same-direction tracking: pick speed from remaining distance to target.
    int speedForForwardDistance(long absDiff) const;
    // While unwinding backlash: pick speed from backlash distance remaining.
    int speedForBacklashRemaining(long remaining) const;

    long maxBacklash() const { return maxBacklash_; }

  private:
    long maxBacklash_;
    SpeedLevels levels_;
    long thHighest_;
    long thHigh_;
    long thMedium_;
    long thLow_;
};
```

`main_sketch/lib/motion/BacklashCompensator.cpp`:
```cpp
#include "BacklashCompensator.h"

BacklashCompensator::BacklashCompensator(long maxBacklash, SpeedLevels levels)
    : maxBacklash_(maxBacklash), levels_(levels) {
    // Mirrors MotorWithEncoder::calculateBackslashRanges().
    if (maxBacklash_ > 50000) {
        thHighest_ = 500; thHigh_ = 200; thMedium_ = 100; thLow_ = 30;
    } else if (maxBacklash_ > 30000) {
        thHighest_ = 500; thHigh_ = 100; thMedium_ = 50; thLow_ = 20;
    } else {
        thHighest_ = 100; thHigh_ = 30; thMedium_ = 10; thLow_ = 5;
    }
}

int BacklashCompensator::speedForForwardDistance(long absDiff) const {
    if (absDiff > 500) return levels_.s5;
    if (absDiff > 200) return levels_.s4;
    if (absDiff > 100) return levels_.s3;
    if (absDiff > 30)  return levels_.s2;
    return levels_.s1;
}

int BacklashCompensator::speedForBacklashRemaining(long remaining) const {
    if (remaining > thHighest_) return levels_.s5;
    if (remaining > thHigh_)    return levels_.s4;
    if (remaining > thMedium_)  return levels_.s3;
    if (remaining > thLow_)     return levels_.s2;
    return levels_.s1;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_motion`
Expected: PASS — 2 tests.

- [ ] **Step 5: Commit**

```bash
git add main_sketch/lib/motion/ main_sketch/test/test_motion/test_backlash.cpp
git commit -m "feat: extract backlash speed-decision logic with native tests"
```

---

### Task 6: `protocol` — parse Android → Arduino commands

Parse an incoming JSON command string into a typed `Command`. Locks the receive half of the Bluetooth contract. Uses ArduinoJson (native-compatible).

**Files:**
- Create: `main_sketch/lib/protocol/Command.h`
- Create: `main_sketch/lib/protocol/Protocol.h`
- Create: `main_sketch/lib/protocol/Protocol.cpp`
- Test: `main_sketch/test/test_protocol/test_parse.cpp`

**Interfaces:**
- Consumes: ArduinoJson.
- Produces:
  - `enum class Cmd { Unknown, Time, Erase, Master, Slave, LightsOn, LightsOff, LaserOn, LaserOff, CStart, CMove, CStop, CSave, CDone, FindStar, Go, MoveStart, Move, MoveDone, MenuMain, MeasureBackslash };`
  - `struct Command { Cmd type; int index; int starIndex; int x; int y; float speed; int raH; int raM; float raS; int decH; int decM; float decS; char time[24]; };`
  - `Command parseCommand(const char* json);`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_protocol/test_parse.cpp`:
```cpp
#include <unity.h>
#include "Protocol.h"
#include <cstring>

void setUp() {}
void tearDown() {}

void test_parse_unknown() {
    Command c = parseCommand("{\"command\":\"bogus\"}");
    TEST_ASSERT_EQUAL(Cmd::Unknown, c.type);
}
void test_parse_simple_command() {
    Command c = parseCommand("{\"command\":\"laser-on\"}");
    TEST_ASSERT_EQUAL(Cmd::LaserOn, c.type);
}
void test_parse_move() {
    Command c = parseCommand("{\"command\":\"move\",\"x\":-40,\"y\":30,\"speed\":5}");
    TEST_ASSERT_EQUAL(Cmd::Move, c.type);
    TEST_ASSERT_EQUAL_INT(-40, c.x);
    TEST_ASSERT_EQUAL_INT(30, c.y);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, c.speed);
}
void test_parse_go() {
    Command c = parseCommand(
        "{\"command\":\"go\",\"ra-h\":6,\"ra-m\":45,\"ra-s\":13.0,"
        "\"dec-h\":-16,\"dec-m\":-45,\"dec-s\":-7.3}");
    TEST_ASSERT_EQUAL(Cmd::Go, c.type);
    TEST_ASSERT_EQUAL_INT(6, c.raH);
    TEST_ASSERT_EQUAL_INT(45, c.raM);
    TEST_ASSERT_EQUAL_INT(-16, c.decH);
}
void test_parse_find_star() {
    Command c = parseCommand("{\"command\":\"find-star\",\"index\":1,\"starIndex\":4}");
    TEST_ASSERT_EQUAL(Cmd::FindStar, c.type);
    TEST_ASSERT_EQUAL_INT(1, c.index);
    TEST_ASSERT_EQUAL_INT(4, c.starIndex);
}
void test_parse_time_string() {
    Command c = parseCommand("{\"command\":\"time\",\"time\":\"2024-03-19 20:31:00.000\"}");
    TEST_ASSERT_EQUAL(Cmd::Time, c.type);
    TEST_ASSERT_EQUAL_STRING("2024-03-19 20:31:00.000", c.time);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_unknown);
    RUN_TEST(test_parse_simple_command);
    RUN_TEST(test_parse_move);
    RUN_TEST(test_parse_go);
    RUN_TEST(test_parse_find_star);
    RUN_TEST(test_parse_time_string);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_protocol`
Expected: FAIL — `parseCommand` undefined.

- [ ] **Step 3: Write the headers and implementation**

`main_sketch/lib/protocol/Command.h`:
```cpp
#pragma once

enum class Cmd {
    Unknown,
    Time, Erase, Master, Slave,
    LightsOn, LightsOff, LaserOn, LaserOff,
    CStart, CMove, CStop, CSave, CDone,
    FindStar, Go,
    MoveStart, Move, MoveDone,
    MenuMain, MeasureBackslash
};

struct Command {
    Cmd type = Cmd::Unknown;
    int index = 0;
    int starIndex = 0;
    int x = 0;
    int y = 0;
    float speed = 0.0f;
    int raH = 0; int raM = 0; float raS = 0.0f;
    int decH = 0; int decM = 0; float decS = 0.0f;
    char time[24] = {0};
};
```

`main_sketch/lib/protocol/Protocol.h`:
```cpp
#pragma once
#include "Command.h"

// Parse a single JSON command object (Android -> Arduino).
Command parseCommand(const char* json);
```

`main_sketch/lib/protocol/Protocol.cpp`:
```cpp
#include "Protocol.h"
#include <ArduinoJson.h>
#include <string.h>

namespace {
Cmd lookup(const char* s) {
    if (!s) return Cmd::Unknown;
    if (!strcmp(s, "time")) return Cmd::Time;
    if (!strcmp(s, "erase")) return Cmd::Erase;
    if (!strcmp(s, "master")) return Cmd::Master;
    if (!strcmp(s, "slave")) return Cmd::Slave;
    if (!strcmp(s, "lights-on")) return Cmd::LightsOn;
    if (!strcmp(s, "lights-off")) return Cmd::LightsOff;
    if (!strcmp(s, "laser-on")) return Cmd::LaserOn;
    if (!strcmp(s, "laser-off")) return Cmd::LaserOff;
    if (!strcmp(s, "c-start")) return Cmd::CStart;
    if (!strcmp(s, "c-move")) return Cmd::CMove;
    if (!strcmp(s, "c-stop")) return Cmd::CStop;
    if (!strcmp(s, "c-save")) return Cmd::CSave;
    if (!strcmp(s, "c-done")) return Cmd::CDone;
    if (!strcmp(s, "find-star")) return Cmd::FindStar;
    if (!strcmp(s, "go")) return Cmd::Go;
    if (!strcmp(s, "move-start")) return Cmd::MoveStart;
    if (!strcmp(s, "move")) return Cmd::Move;
    if (!strcmp(s, "move-done")) return Cmd::MoveDone;
    if (!strcmp(s, "menu-main")) return Cmd::MenuMain;
    if (!strcmp(s, "measure-backslash")) return Cmd::MeasureBackslash;
    return Cmd::Unknown;
}
}  // namespace

Command parseCommand(const char* json) {
    Command c;
    JsonDocument doc;
    if (deserializeJson(doc, json)) {
        return c;  // parse error -> Unknown
    }
    c.type = lookup(doc["command"] | (const char*)nullptr);

    c.index = doc["index"] | 0;
    c.starIndex = doc["starIndex"] | 0;
    c.x = doc["x"] | 0;
    c.y = doc["y"] | 0;
    c.speed = doc["speed"] | 0.0f;
    c.raH = doc["ra-h"] | 0;
    c.raM = doc["ra-m"] | 0;
    c.raS = doc["ra-s"] | 0.0f;
    c.decH = doc["dec-h"] | 0;
    c.decM = doc["dec-m"] | 0;
    c.decS = doc["dec-s"] | 0.0f;

    const char* t = doc["time"] | (const char*)nullptr;
    if (t) {
        strncpy(c.time, t, sizeof(c.time) - 1);
        c.time[sizeof(c.time) - 1] = '\0';
    }
    return c;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_protocol`
Expected: PASS — 6 tests.

- [ ] **Step 5: Commit**

```bash
git add main_sketch/lib/protocol/ main_sketch/test/test_protocol/test_parse.cpp
git commit -m "feat: parse Android->Arduino JSON commands (protocol contract)"
```

---

### Task 7: `protocol` — build Arduino → Android telemetry

Serialize a `TelemetryState` to the exact telemetry JSON the Android app parses. Locks the transmit half of the contract.

**Files:**
- Modify: `main_sketch/lib/protocol/Protocol.h`
- Modify: `main_sketch/lib/protocol/Protocol.cpp`
- Create: `main_sketch/lib/protocol/Telemetry.h`
- Test: `main_sketch/test/test_protocol/test_telemetry.cpp`

**Interfaces:**
- Consumes: ArduinoJson.
- Produces:
  - `struct TelemetryState { double ra; double dec; double lastStarRa; double lastStarDec; double azm; double alt; double cmAzm; double cmAlt; int calibrated; int activeMode; long horMotor; long verMotor; long horEncoder; long verEncoder; double accX; double accY; double accZ; };`
  - `size_t buildTelemetry(const TelemetryState& s, char* out, size_t outSize);`

- [ ] **Step 1: Write the failing test**

`main_sketch/test/test_protocol/test_telemetry.cpp`:
```cpp
#include <unity.h>
#include "Protocol.h"
#include "Telemetry.h"
#include <ArduinoJson.h>

void setUp() {}
void tearDown() {}

void test_telemetry_roundtrip_keys() {
    TelemetryState s{};
    s.ra = 10.5; s.dec = -20.25;
    s.lastStarRa = 6.0; s.lastStarDec = -16.0;
    s.azm = 123.4; s.alt = 45.6;
    s.cmAzm = 120.0; s.cmAlt = 44.0;
    s.calibrated = 1; s.activeMode = 4;
    s.horMotor = 111; s.verMotor = 222;
    s.horEncoder = 333; s.verEncoder = 444;
    s.accX = 0.1; s.accY = 0.2; s.accZ = 1.0;

    char buf[512];
    size_t n = buildTelemetry(s, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);

    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, buf));
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 10.5, (double)doc["ra"]);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, -20.25, (double)doc["dec"]);
    TEST_ASSERT_EQUAL_INT(1, (int)doc["calibrated"]);
    TEST_ASSERT_EQUAL_INT(4, (int)doc["activeMode"]);
    TEST_ASSERT_EQUAL_INT64(333, (long)doc["hor-encoder"]);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 0.2, (double)doc["acc-y"]);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 120.0, (double)doc["c-m-azm"]);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_telemetry_roundtrip_keys);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd main_sketch && pio test -e native -f test_protocol`
Expected: FAIL — `TelemetryState` / `buildTelemetry` undefined.

- [ ] **Step 3: Write the header and implementation**

`main_sketch/lib/protocol/Telemetry.h`:
```cpp
#pragma once

struct TelemetryState {
    double ra = 0, dec = 0;
    double lastStarRa = 0, lastStarDec = 0;
    double azm = 0, alt = 0;
    double cmAzm = 0, cmAlt = 0;
    int calibrated = 0;
    int activeMode = 0;
    long horMotor = 0, verMotor = 0;
    long horEncoder = 0, verEncoder = 0;
    double accX = 0, accY = 0, accZ = 0;
};
```

Append to `main_sketch/lib/protocol/Protocol.h`:
```cpp
#include "Telemetry.h"

// Serialize telemetry (Arduino -> Android). Returns bytes written (excl. NUL).
size_t buildTelemetry(const TelemetryState& s, char* out, size_t outSize);
```

Append to `main_sketch/lib/protocol/Protocol.cpp`:
```cpp
size_t buildTelemetry(const TelemetryState& s, char* out, size_t outSize) {
    JsonDocument doc;
    doc["ra"] = s.ra;
    doc["dec"] = s.dec;
    doc["last-star-ra"] = s.lastStarRa;
    doc["last-star-dec"] = s.lastStarDec;
    doc["azm"] = s.azm;
    doc["alt"] = s.alt;
    doc["c-m-azm"] = s.cmAzm;
    doc["c-m-alt"] = s.cmAlt;
    doc["calibrated"] = s.calibrated;
    doc["activeMode"] = s.activeMode;
    doc["hor-motor"] = s.horMotor;
    doc["ver-motor"] = s.verMotor;
    doc["hor-encoder"] = s.horEncoder;
    doc["ver-encoder"] = s.verEncoder;
    doc["acc-x"] = s.accX;
    doc["acc-y"] = s.accY;
    doc["acc-z"] = s.accZ;
    return serializeJson(doc, out, outSize);
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd main_sketch && pio test -e native -f test_protocol`
Expected: PASS — parse + telemetry tests.

- [ ] **Step 5: Run the full native suite**

Run: `cd main_sketch && pio test -e native`
Expected: PASS — all suites (`test_smoke`, `test_core`, `test_astro`, `test_motion`, `test_protocol`).

- [ ] **Step 6: Commit**

```bash
git add main_sketch/lib/protocol/Telemetry.h main_sketch/lib/protocol/Protocol.h main_sketch/lib/protocol/Protocol.cpp main_sketch/test/test_protocol/test_telemetry.cpp
git commit -m "feat: build Android telemetry JSON (protocol contract, tx half)"
```

---

## Self-Review

**Spec coverage (Plan 1 slice):**
- Modular `lib/` structure with pure logic → Tasks 2–7 (`core`, `astro`, `motion`, `protocol`). ✅
- Native test harness (no hardware) → Task 1. ✅
- Bluetooth protocol contract (commands + telemetry keys) → Tasks 6–7, verified against the Global Constraints list. ✅
- Calibration + backlash logic preserved → Tasks 4–5 (ported verbatim from `MotorWithEncoder`/`main_sketch.ino`). ✅
- HAL (MotorAxis/PCNT/ADS/MCP/OLED), `BluetoothLink`, NVS, Ephemeris wrapper, `State` object, `main.cpp` → **deferred to Plans 2–5** (hardware-dependent), noted in the roadmap. Not gaps in this plan.

**Placeholder scan:** none — every step has runnable code/commands.

**Type consistency:** `mapDouble` signature identical across Tasks 2/3/4; `Command`/`Cmd` names consistent Tasks 6→7; `TelemetryState` field names match the telemetry keys in Global Constraints.

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-07-01-esp32-port-foundation.md`. Two execution options:

**1. Subagent-Driven (recommended)** — I dispatch a fresh subagent per task, review between tasks, fast iteration.

**2. Inline Execution** — Execute tasks in this session using executing-plans, batch execution with checkpoints.

Which approach?
