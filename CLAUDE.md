# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MarlinBio is a fork of [Marlin firmware](https://github.com/MarlinFirmware/Marlin) customized for the **Printess 2.0** open-source bioprinter. Many original Marlin features have been removed or heavily refactored to serve bioprinting needs (compression syringe extrusion, thermoelectric temperature control, multi-Z-axis tool systems). Do not assume standard Marlin behavior for modified subsystems.

**Target hardware:** BigTreeTech Octopus Max EZ V1.0 (STM32H723ZE). Only TMC2209 stepper drivers are supported.

## Build Commands

```bash
# Build firmware (default env: STM32H723ZE_btt)
pio run

# Build a specific environment
pio run -e STM32H723ZE_btt

# Build with debug
pio run -e STM32H723ZE_btt_debug

# Clean build
pio run -t clean

# Run all unit tests
make unit-test-all-local
# or: platformio run -t test-marlin -e linux_native_test

# Run a single unit test config
make unit-test-single-local UNIT_TEST_CONFIG=default
# or: platformio run -t marlin_default -e linux_native_test

# Run integration tests for a specific target
make tests-single-local TEST_TARGET=<target>

# Run all integration tests
make tests-all-local
```

The build produces `firmware.bin` which is flashed by copying to a microSD card inserted into the control board.

## Architecture

This is a PlatformIO-based C++ embedded firmware project. The source lives under `Marlin/src/`.

### Key Directories

- `Marlin/Configuration.h` — **Primary config file.** Refactored and simplified from Marlin. Contains all user-facing settings (motion, extrusion, temperature, motors, sensorless homing). `Configuration_adv.h` is now empty; all settings are consolidated here.
- `Marlin/src/module/` — Core firmware modules: `stepper.cpp`, `planner.cpp`, `motion.cpp`, `temperature.cpp`, `tool_change.cpp`, `endstops.cpp`, `settings.cpp`
- `Marlin/src/gcode/` — G-code command implementations, organized by category (motion, temp, feature, calibrate, control, host)
- `Marlin/src/feature/` — Optional features: `mixing.cpp` (mixing extruders), `tmc_util.cpp` (TMC driver utilities)
- `Marlin/src/gcode/feature/constant_extrusion/M789.cpp` — Custom M789 command for constant extrusion
- `Marlin/src/inc/` — Conditional compilation logic (`Conditionals-*.h`), config preprocessing, sanity checks
- `Marlin/src/HAL/` — Hardware Abstraction Layer (STM32 is the relevant target)
- `Marlin/src/core/` — Low-level types, macros, board definitions
- `ini/` — PlatformIO environment configs per MCU family; `stm32h7.ini` contains the target board env
- `platformio.ini` — Main build config. Note the `default_src_filter` which explicitly includes/excludes source files

### Printess-Specific Subsystems (diverge significantly from upstream Marlin)

1. **Tool change system** (`Marlin/src/module/tool_change.cpp`): Uses `T` command to switch between up to 4 extruder systems, each with its own Z axis. Only X/Y/Z/E labels are used (no A/B/C/I/J/K/U/V/W). Nozzle offsets via `M218`, Z raise via `M217`.

2. **Constant extrusion** (`M789.cpp`, integrated into `planner.cpp`, `stepper.cpp`, `motion.cpp`): Sets extruder motor speed based on `Speed = feedrate * K * (D_needle / D_syringe)` during print moves. Includes pressurization/depressurization. The `E` axis label is ignored during print moves when enabled.

3. **Mixing extruders** (`Marlin/src/feature/mixing.cpp`): Completely reworked from Marlin's original. Links sequential extruders into one tool with configurable mix ratios. Configured via `MIXING_CONFIGURATION` define. `M163` sets ratios; `M164`/`M165` are not supported.

4. **Temperature control** (`Marlin/src/module/temperature.cpp`, `Marlin/src/gcode/temp/M104_M109.cpp`): Redone for thermoelectric (Peltier) modules that can heat OR cool. `M104` has been completely rewritten (uses `I` for module index, `C`/`H`/`D` for cool/heat/disable). `M109` is not supported. Includes custom thermal safety monitors and auto fan control with dampening.

### Build System Details

- PlatformIO manages dependencies and board configs
- `platformio.ini` has a selective source filter — new source files must be explicitly added to `default_src_filter` or they won't compile
- The `ini/features.ini` file maps `#define` flags to source file includes
- `Marlin/config.ini` can override Configuration.h settings pre-build (currently set to `none`)
- Test configs live in `test/` as `.ini` files

### Conditional Compilation

The firmware uses extensive `#if ENABLED(...)` / `#if DISABLED(...)` preprocessor guards. The define-to-feature mapping flows through:
1. `Configuration.h` (user defines)
2. `Conditionals-*.h` chain (derived defines)
3. `SanityCheck.h` (validation)
4. `ini/features.ini` (source file inclusion)

When adding features, ensure the corresponding source files are included in both `features.ini` and `platformio.ini`'s `default_src_filter`.
