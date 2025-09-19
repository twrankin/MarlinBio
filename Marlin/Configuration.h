/*
 * MarlinBio 3D Printer Firmware
 * Copyright (c) 2025 MarlinBio [https://github.com/twrankin/MarlinBio]
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

/*
 * Configuration.h has been refactored for Printess 2.0.
 * It is much shorter and many irrelevant settings have been removed.
 * If you rely on a traditional Marlin setting for your prints, you can try
 * redefining it; however, the associated feature is highly likely to be broken as
 * a result of our changes. If you test a removed feature that works, let us know and we'll
 * add it back; if you test a removed feature that's broken, and you'd like us to fix it, open
 * an issue or discussion on github with a compelling use case, and we'll look into it:
 * https://github.com/twrankin/MarlinBio/discussions.
 */

//=============================================================================
//================================== Motion ===================================
//=============================================================================

/*
 * Default axis mode.
 * true initializes an axis to relative mode (G91).
 * false initializes an axis to absolute mode (G90).
 * X, Y, Z, E.
 */
 #define AXIS_RELATIVE_MODES { true, true, true, true }

/*
 * With this option, each E stepper can have its own factors for some
 * of the movement settings. If fewer factors are given than the
 * total number of extruders, the last value applies to the rest.
 */
//#define DISTINCT_E_FACTORS

/*
 * Default axis steps per unit (steps/mm).
 * This is how many steps will be issued to move an axis 1mm.
 * If the MICROSTEPS setting is changed, this needs to be multiplied
 * accordingly. E.g. E = 256usteps/steps * 200steps/mm = 51200 usteps/mm.
 * Override with M92.
 * X, Y, Z, E0 [, E1[, E2...]].
 */
 #define DEFAULT_AXIS_STEPS_PER_UNIT { 200, 200, 200, 51200 }

/*
 * Default max feed rate (mm/s).
 * Too high of a feedrate will cause stalling.
 * Override with M203.
 * X, Y, Z, E0 [, E1[, E2...]].
 */
#define DEFAULT_MAX_FEEDRATE { (600 / 60), (600 / 60), (600 / 60), (600 / 60) }

/*
 * Default max acceleration (mm/s^2).
 * Too high of an acceleration will cause stalling.
 * Override with M201.
 * X, Y, Z, E0 [, E1[, E2...]].
 */
#define DEFAULT_MAX_ACCELERATION { 500, 500, 500, 500 }

/*
 * Default acceleration (mm/s^2).
 * Override with M204.
 */

// X, Y, Z and E acceleration for printing moves.
#define DEFAULT_ACCELERATION 500
// Acceleration when only E is specified.
#define DEFAULT_RETRACT_ACCELERATION 500
// X, Y, Z acceleration for travel (non printing) moves.
#define DEFAULT_TRAVEL_ACCELERATION 500

// Invert the stepper direction if an axis goes the wrong way.
#define INVERT_X_DIR  true
#define INVERT_Y_DIR  true
#define INVERT_Z_DIR  false
#define INVERT_E0_DIR true
#define INVERT_E1_DIR true
#define INVERT_E2_DIR true
#define INVERT_E3_DIR true

// Travel limits.
#define X_MIN_POS 0
#define X_MAX_POS 150
#define Y_MIN_POS 0
#define Y_MAX_POS 100
#define Z_MIN_POS 0
#define Z_MAX_POS 100

/*
 * Software endstops prevent moves outside of the set machine bounds.
 * Use M211 to set software endstops on/off or report the current state.
 */
#define MIN_SOFTWARE_ENDSTOPS
#if ENABLED(MIN_SOFTWARE_ENDSTOPS)
  #define MIN_SOFTWARE_ENDSTOP_X
  #define MIN_SOFTWARE_ENDSTOP_Y
  #define MIN_SOFTWARE_ENDSTOP_Z
#endif

#define MAX_SOFTWARE_ENDSTOPS
#if ENABLED(MAX_SOFTWARE_ENDSTOPS)
  #define MAX_SOFTWARE_ENDSTOP_X
  #define MAX_SOFTWARE_ENDSTOP_Y
  #define MAX_SOFTWARE_ENDSTOP_Z
#endif

// Minimum Z height before homing (to make sure the nozzle doesn't hit anything).
#define Z_CLEARANCE_FOR_HOMING 0

// Height to move to after homing Z.
//#define Z_AFTER_HOMING 10
// Position to move to after homing XY.
//#define XY_AFTER_HOMING { 10, 10 }

// Commands to run after G28.
//#define EVENT_GCODE_AFTER_HOMING "M300 P440 S200"

// Direction of endstops when homing; 1 = MAX, -1 = MIN.
#define X_HOME_DIR -1
#define Y_HOME_DIR -1
#define Z_HOME_DIR  1

// Homing speeds (mm/min).
#define HOMING_FEEDRATE_MM_M { 300, 300, 300 }

// This validates that the endstops are triggered on homing moves.
// Disable this if the kill on failed homing becomes a nuisance.
#define VALIDATE_HOMING_ENDSTOPS

/*
 * G2/G3 arc settings.
 */
#define ARC_SUPPORT
#if ENABLED(ARC_SUPPORT)
  // Minimum and maximum length of each arc segment in mm.
  #define MIN_ARC_SEGMENT_MM 0.1
  #define MAX_ARC_SEGMENT_MM 1.0

  // Minimum number of segments in a complete circle.
  #define MIN_CIRCLE_SEGMENTS 72

  // Use the feedrate to choose the segment length.
  //#define ARC_SEGMENTS_PER_SEC 50

  // Number of interpolated segments between corrections.
  #define N_ARC_CORRECTION 25

  // Enable the 'P' parameter to specify complete circles.
  //#define ARC_P_CIRCLES
#endif

//===========================================================================
//================================ Extrusion ================================
//===========================================================================

/*
 * This defines the number of extruders (and correspondingly the number of Z axes); 0, 1, 2, 3, or 4.
 * Some per-extruder parameters will need to be updated when EXTRUDERS is changed.
 * When enabling MIXING_EXTRUDERS, still use the actual number of extruders here.
 */
#define EXTRUDERS 4

/*
 * The following are parameters for constant extrusion based on a syringe extruder system.
 * When constant extrusion is enabled, the speed of the current extruder motor(s) is statically
 * set during a print move (G1/G2/G3), and runs constantly until the next non-print move or
 * until no more print moves are queued. Constant extrusion also uses small movements to
 * pressurize before print moves and subsequently depressurize before the next non-print move.
 * We determine the speed of extrusion based on the equation:
 * Speed * (π * syringe_diam^2 / 4) = feedrate * K * (π * needle_diam^2 / 4),
 * i.e., what is pushed out of the syringe comes out of the needle at a different speed.
 * The correction coefficient, K, is essentially the means to alter this speed and consequently
 * the width of the extruded line. Each parameter is an array of values for each extruder.
 */

// Constant extrusion can be enabled by default here, and enabled/disabled live by the M789 command.
#define CONSTANT_EXTRUSION_DEFAULT_ON

// The inner diameter of the syringe in mm.
#define SYRINGE_INNER_DIAMETER { 4.78, 4.78, 4.78, 4.78 }

// The inner diameter of the needle in mm.
#define NEEDLE_INNER_DIAMETER { 0.838, 0.838, 0.838, 0.838 }

// The coefficient to scale the extrusion rate.
#define EXTRUSION_COEFFICIENT { 2.0, 2.0, 2.0, 2.0 }

// The distance to pressurize/depressurize the syringe before/after printing in mm.
#define PRESSURIZATION_LENGTH { 0.5, 0.5, 0.5, 0.5 }

/*
 * This enables the use of mixing materials together using a mixing module or printhead to extrude
 * out of one nozzle. Linking extruders into one tool will cause them to both move when extruder
 * movement is specified for that tool, scaled by the ratios specified here. Extruders in a mix
 * are assumed to be physically connected; as such, their Z axes will be moved in unison at all times.
 * The original MarlinFirmware implementation of mixing extruders was not sufficient for our needs
 * and has been completely reworked. The M164 and M165 commands are not supported. Gradient
 * functionality will be added in the future, let us know if this is needed sooner.
 * Make sure to update the nozzle offsets below.
 */
//#define MIXING_EXTRUDER
#if ENABLED(MIXING_EXTRUDER)
  // The configuration of mixing extruders is specified as an array of arrays,
  // where the inner arrays link extruders together. Linked extruders must be sequential,
  // { { 0, 2 } } is not a valid combination, but { { 0, 1, 2 } } is.
  // Unlisted extruders will operate as normal.
  #define MIXING_CONFIGURATION { { 1, 2 } }
  // The mixing ratio for each linked extruder is specified in the same format
  // as the configuration. The ratio for an extruder simply scales the distance or speed
  // of that extruder when moving. For example, if extruders 0 and 1 are linked with ratios
  // of 1 and 0.5, an extrusion of 10mm for tool 0 would move extruder 0 by 10mm and extruder
  // 1 by 5mm. Ratios can be changed live using the M163 G-code command.
  // I.E. "M163 E0 R0.75" would set extruder 0's ratio to 0.75.
  #define MIX_RATIOS { { 1.0, 0.5 } }
#endif

#if EXTRUDERS > 1
  /*
   * The offset of each nozzle in relation to nozzle 0 in mm. Defining these will move the X/Y/Z
   * axes according to the offsets when changing tools. The offsets for nozzle 0 must all be zero.
   * The number of nozzles is calculated using the number of extruders and their linkages (if using MIXING_EXTRUDER).
   * E.g., 4 extruders with one linked pair is 3 nozzles. These can be altered live using the M218 G-code command.
   * WARNING: If using offsets, the specified axes must be homed before a tool change!
   */
  //#define NOZZLE_OFFSET_X { 0.0, -30.0, -60.0, -90.0 }
  //#define NOZZLE_OFFSET_Y { 0.0, 0.2, 0.0, -0.1 }
  //#define NOZZLE_OFFSET_Z { 0.0, -0.23, -0.05, 1.0}

  /*
   * The absolute Z position to raise the current Z axis to before changing tools.
   * The new Z axis will lower down to the previous position. This can be changed
   * live using the M217 Z command. The other M217 parameters are not supported.
   * If this is non-zero, the Z axes must have been homed at least once before a tool change.
   */
  #define TOOLCHANGE_ZRAISE Z_MAX_POS

  // Extra G-code to run after specific tool changes.
  //#define EVENT_GCODE_TOOLCHANGE_T0 "G28 A\nG1 A0"
  //#define EVENT_GCODE_TOOLCHANGE_T1 "G1 A10"

  // Extra G-code to run after all tool changes.
  //#define EVENT_GCODE_AFTER_TOOLCHANGE "G12X"

  /*
   * Consider coordinates for EVENT_GCODE_TOOLCHANGE_Tx as relative to T0
   * so that moves in the specified axes are the same for all tools.
   * This uses NOZZLE_OFFSET_... for the reference.
   */
  //#define TC_GCODE_USE_GLOBAL_X
  //#define TC_GCODE_USE_GLOBAL_Y
  //#define TC_GCODE_USE_GLOBAL_Z
#endif // EXTRUDERS > 1

//===========================================================================
//================================ Temperature ==============================
//===========================================================================

/*
 * The temperature system has been redone to support our custom temperature control
 * modules that utilize thermoelectric coolers.
 * The temperature modules start in a disabled state. To enable them to heat or cool,
 * set a target temperature, or disable them when finished, use the M104 command.
 * It is not recommended to switch between heating and cooling while the device is powered,
 * as this requires rewiring, but can be done by first disabling the module.
 * The M109 command is not supported.
 * Module 4 is the bed.
 * WARNING: The peltier must be connected in the correct direction for heating or cooling.
 * WARNING: Always rewire with the power completely disconnected.
 */

// A fatal error will be issued below this temperature.
#define MODULE_0_MINTEMP -15
#define MODULE_1_MINTEMP -15
#define MODULE_2_MINTEMP -15
#define MODULE_3_MINTEMP -15
#define MODULE_4_MINTEMP -15

// A fatal error will be issued above this temperature.
#define MODULE_0_MAXTEMP 75
#define MODULE_1_MAXTEMP 75
#define MODULE_2_MAXTEMP 75
#define MODULE_3_MAXTEMP 75
#define MODULE_4_MAXTEMP 75

/*
 * During operation, the temperature can often overshoot the target by many degrees.
 * Setting the target temperature too close to MAXTEMP/MINTEMP guarantees a shutdown!
 * Use this value to block temperatures being set over/under MAXTEMP/MINTEMP -/+ OVERSHOOT.
 */
#define TEMPERATURE_OVERSHOOT 15

/*
 * Dampen the fans upon reaching the cooling target.
 * When auto fans are enabled, the fans will run at a certain speed depending on the target
 * temperature. Once the target is reached and the temperature only needs to be maintained,
 * the fan speed can be reduced for noise tolerance. This is the PWM that will be set upon
 * reaching the target. Set it lower if the noise is bothersome, and higher if the conditions
 * cause the temperature to become unstable. M106 can also be used to disable the auto fan feature
 * entirely and allow manual control.
 * 0 - 255.
 */
#define AUTO_FAN_DAMPEN 128

// This should really never be disabled.
#define MODULE_THERMAL_PROTECTION
#if ENABLED(MODULE_THERMAL_PROTECTION)
  /*
   * Whenever an M104 command changes the target temperature, the temperature will be
   * monitored for WATCH_HEAT/COOL_PERIOD seconds. If the temperature doesn't
   * increase/decrease by WATCH_TEMP_INCREASE/DECREASE degrees, the machine will be
   * halted as a safety precaution. This process repeats until the temperature is
   * close to the target.
   *
   * If you get false errors for "Heating/Cooling failed", increase WATCH_HEAT/COOL_PERIOD
   * and/or decrease WATCH_TEMP_INCREASE/DECREASE, but beware the hazard of doing so.
   */

  #define WATCH_HEAT_PERIOD   30
  #define WATCH_COOL_PERIOD   60
  #define WATCH_TEMP_INCREASE 2
  #define WATCH_TEMP_DECREASE 0.5

  /*
   * Once the temperature reaches the target, it will be monitored for stability.
   * If the temperature stays too far off target for too long, the machine will be
   * halted as a safety precaution.
   *
   * If you get false errors for "Thermal Runaway", increase THERMAL_PROTECTION_PERIOD
   * and/or THERMAL_PROTECTION_HYSTERESIS, but beware the hazard of doing so.
   */
  #define THERMAL_PROTECTION_PERIOD 40
  #define THERMAL_PROTECTION_HYSTERESIS 4

  /*
   * This feature attempts to detect stuck sensors.
   *
   * Within the monitoring period, at least one temperature change must occur,
   * to indicate that sensor polling is working. If any monitored temperature remains
   * totally constant (without even a fractional change), a thermal malfunction error
   * occurs and the printer is halted as a safety precaution.
   *
   * Be careful to distinguish false positives from real sensor issues before disabling this feature.
   */
  #define THERMAL_PROTECTION_VARIANCE_MONITOR
  #if ENABLED(THERMAL_PROTECTION_VARIANCE_MONITOR)
    // By default, this uses the THERMAL_PROTECTION_PERIOD constant above for the time window.
    // This setting will override it for a longer period.
    //#define THERMAL_PROTECTION_VARIANCE_MONITOR_PERIOD 30
  #endif
#endif

//=============================================================================
//=================================== Motors ==================================
//=============================================================================

// Only TMC2209 has been tested and is supported. Other drivers may work but are not guaranteed.
#define USING_TMC220x
#if ENABLED(USING_TMC220x)
  /*
   * For the following settings:
   * - x_CURRENT: The current in mA used to drive the stepper motor.
   *   It should match the specification for the chosen motor.
   * 
   * - x_CURRENT_HOME: The current in mA used during homing.
   *   Using a slightly lower value can prevent false positives.
   * 
   * - x_MICROSTEPS: Set this parameter to divide a full step into 0, 2, 4, ..., 256 micro steps.
   *   E.g., if x_MICROSTEPS is set to 16, then one step pulse is equal to 1/16th of a full step.
   *   If this is changed, DEFAULT_AXIS_STEPS_PER_UNIT needs to be multiplied accordingly.
   */
  #define X_CURRENT       600
  #define X_CURRENT_HOME  450
  #define X_MICROSTEPS    0

  #define Y_CURRENT       600
  #define Y_CURRENT_HOME  450
  #define Y_MICROSTEPS    0

  #define Z_CURRENT       600
  #define Z_CURRENT_HOME  450
  #define Z_MICROSTEPS    0

  #define Z2_CURRENT      Z_CURRENT
  #define Z2_CURRENT_HOME Z_CURRENT_HOME
  #define Z2_MICROSTEPS   Z_MICROSTEPS

  #define Z3_CURRENT      Z_CURRENT
  #define Z3_CURRENT_HOME Z_CURRENT_HOME
  #define Z3_MICROSTEPS   Z_MICROSTEPS

  #define Z4_CURRENT      Z_CURRENT
  #define Z4_CURRENT_HOME Z_CURRENT_HOME
  #define Z4_MICROSTEPS   Z_MICROSTEPS

  // High E microsteps allows for smoother constant extrusion.
  #define E0_CURRENT      500
  #define E0_MICROSTEPS   256

  #define E1_CURRENT      E0_CURRENT
  #define E1_MICROSTEPS   E0_MICROSTEPS

  #define E2_CURRENT      E0_CURRENT
  #define E2_MICROSTEPS   E0_MICROSTEPS

  #define E3_CURRENT      E0_CURRENT
  #define E3_MICROSTEPS   E0_MICROSTEPS

  /*
   * Sensorless homing uses a feature on TMC drivers that detects when a
   * motor has stalled based on the back-EMF. This enables use of the G28
   * command without an encoder. Sensitivity values are 0-255.
   * If the value is too low, it won't detect a stall, and the motor will
   * buzz as it torques into the endstop. If the value is too high it will
   * detect false positives and end homing too soon. Tweaking will be necessary,
   * as the operable range changes based on load, microsteps, motor lubrication,
   * looking at it wrong, etc.. Use M914 to change the sensitivity at runtime.
   */
  #define SENSORLESS_HOMING
  #if ENABLED(SENSORLESS_HOMING)
    #define X_STALL_SENSITIVITY  125
    #define Y_STALL_SENSITIVITY  125
    #define Z_STALL_SENSITIVITY  125
    #define Z2_STALL_SENSITIVITY 125
    #define Z3_STALL_SENSITIVITY 125
    #define Z4_STALL_SENSITIVITY 125
  #endif
#endif

//===========================================================================
//================================== Extra ==================================
//===========================================================================

// Execute G-code commands immediately after power-on.
//#define STARTUP_COMMANDS "M17 Z"

// Add G-codes M810-M819 to define and run G-code macros.
//#define GCODE_MACROS
#if ENABLED(GCODE_MACROS)
  #define GCODE_MACROS_SLOTS     5
  #define GCODE_MACROS_SLOT_SIZE 50
#endif

//===========================================================================
//================================ Advanced =================================
//===========================================================================
/*
 * WARNING: The below settings are advanced and shouldn't be touched unless the user knows exactly
 * what they are doing. If any of the below settings are modified, breaking and potentially unsafe
 * issues are at a high risk of occurring.
*/

#define DIAG_JUMPERS_REMOVED
#define NO_USER_FEEDBACK_WARNING
#define NO_AUTO_ASSIGN_WARNING

#define CONFIGURATION_H_VERSION 02010300
#define STRING_CONFIG_H_AUTHOR "MarlinBio"
#ifndef MOTHERBOARD
  #define MOTHERBOARD BOARD_BTT_OCTOPUS_MAX_EZ_V1_0
#endif
#define CUSTOM_MACHINE_NAME "Printess 2.0"

#define SERIAL_PORT -1
#define BAUDRATE 250000
#define MAX_CMD_SIZE 96
#define BUFSIZE 4
#define RX_BUFFER_MONITOR
#define SERIAL_OVERRUN_PROTECTION
#define PROPORTIONAL_FONT_RATIO 1.0

#define X_DRIVER_TYPE  TMC2209
#define Y_DRIVER_TYPE  TMC2209
#define Z_DRIVER_TYPE  TMC2209
#define Z2_DRIVER_TYPE TMC2209
#define Z3_DRIVER_TYPE TMC2209
#define Z4_DRIVER_TYPE TMC2209
#define E0_DRIVER_TYPE TMC2209
#define E1_DRIVER_TYPE TMC2209
#define E2_DRIVER_TYPE TMC2209
#define E3_DRIVER_TYPE TMC2209

#define DEFAULT_NOMINAL_FILAMENT_DIA 1.75

#define TEMP_SENSOR_0 61
#define TEMP_SENSOR_1 61
#define TEMP_SENSOR_2 61
#define TEMP_SENSOR_3 61
#define TEMP_SENSOR_4 61
#define BANG_MAX 255
#if HAS_E_TEMP_SENSOR
  #define TEMP_RESIDENCY_TIME 10
  #define TEMP_WINDOW 1
  #define TEMP_HYSTERESIS 3
#endif

#define ENDSTOPPULLUPS
#define X_MIN_ENDSTOP_HIT_STATE HIGH
#define X_MAX_ENDSTOP_HIT_STATE HIGH
#define Y_MIN_ENDSTOP_HIT_STATE HIGH
#define Y_MAX_ENDSTOP_HIT_STATE HIGH
#define Z_MIN_ENDSTOP_HIT_STATE HIGH
#define Z_MAX_ENDSTOP_HIT_STATE HIGH
#define I_MIN_ENDSTOP_HIT_STATE HIGH
#define I_MAX_ENDSTOP_HIT_STATE HIGH
#define J_MIN_ENDSTOP_HIT_STATE HIGH
#define J_MAX_ENDSTOP_HIT_STATE HIGH
#define K_MIN_ENDSTOP_HIT_STATE HIGH
#define K_MAX_ENDSTOP_HIT_STATE HIGH
#define U_MIN_ENDSTOP_HIT_STATE HIGH
#define U_MAX_ENDSTOP_HIT_STATE HIGH
#define V_MIN_ENDSTOP_HIT_STATE HIGH
#define V_MAX_ENDSTOP_HIT_STATE HIGH
#define W_MIN_ENDSTOP_HIT_STATE HIGH
#define W_MAX_ENDSTOP_HIT_STATE HIGH
#define Z_MIN_PROBE_ENDSTOP_HIT_STATE HIGH
#define Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN

#define EDITABLE_STEPS_PER_UNIT
#define JUNCTION_DEVIATION_MM 0.013
#define JD_HANDLE_SMALL_SEGMENTS

#define X_ENABLE_ON LOW
#define Y_ENABLE_ON LOW
#define Z_ENABLE_ON LOW
#define E_ENABLE_ON LOW

#define EDITABLE_HOMING_FEEDRATE

#define EEPROM_CHITCHAT
#define EEPROM_BOOT_SILENT

#define HOST_KEEPALIVE_FEATURE
#define DEFAULT_KEEPALIVE_INTERVAL 2
#define BUSY_WHILE_HEATING

#define PRINTJOB_TIMER_AUTOSTART

#define HOMING_BUMP_MM      { 0, 0, 0 }
#define HOMING_BUMP_DIVISOR { 2, 2, 4 }
#define QUICK_HOME
#define HOME_Z_FIRST

#define STEP_STATE_X HIGH
#define STEP_STATE_Y HIGH
#define STEP_STATE_Z HIGH
#define STEP_STATE_E HIGH

#define DEFAULT_STEPPER_TIMEOUT_SEC 120
#define DISABLE_IDLE_X
#define DISABLE_IDLE_Y
#define DISABLE_IDLE_Z
#define DISABLE_IDLE_E

#define DEFAULT_MINIMUMFEEDRATE 0.0
#define DEFAULT_MINTRAVELFEEDRATE 0.0
#define DEFAULT_MINSEGMENTTIME 20000

#define SLOWDOWN
#if ENABLED(SLOWDOWN)
  #define SLOWDOWN_DIVISOR 2
#endif

#define MULTISTEPPING_LIMIT 16

#define GCODE_REPEAT_MARKERS

#define SDSUPPORT
#if ENABLED(SDSUPPORT)
  #define SD_PROCEDURE_DEPTH 1
  #define SD_FINISHED_STEPPERRELEASE true
  #define SD_FINISHED_RELEASECOMMAND "M84"
  #define SDCARD_RATHERRECENTFIRST
  #define BINARY_FILE_TRANSFER
  #if ENABLED(BINARY_FILE_TRANSFER)
    #define CUSTOM_FIRMWARE_UPLOAD
  #endif
#endif

#define USE_WATCHDOG

#define MIN_STEPS_PER_SEGMENT 6

#define BLOCK_BUFFER_SIZE 16

#define AUTO_REPORT_TEMPERATURES

#define CAPABILITIES_REPORT
#define EXTENDED_CAPABILITIES_REPORT

#define FASTER_GCODE_PARSER
#define DEBUG_FLAGS_GCODE

#define HOST_ACTION_COMMANDS

#define CONFIGURATION_ADV_H_VERSION 02010300

#ifdef Z2_DRIVER_TYPE
  #define Z_MULTI_ENDSTOPS
  #if ENABLED(Z_MULTI_ENDSTOPS)
    #define Z2_STOP_PIN Z2_DIAG_PIN
    #define Z2_ENDSTOP_ADJUSTMENT 0
  #endif
  #ifdef Z3_DRIVER_TYPE
    #if ENABLED(Z_MULTI_ENDSTOPS)
      #define Z3_STOP_PIN E0_DIAG_PIN
      #define Z3_ENDSTOP_ADJUSTMENT 0
    #endif
  #endif
  #ifdef Z4_DRIVER_TYPE
    #if ENABLED(Z_MULTI_ENDSTOPS)
      #define Z4_STOP_PIN E1_DIAG_PIN
      #define Z4_ENDSTOP_ADJUSTMENT 0
    #endif
  #endif
#endif

#if ENABLED(USING_TMC220x)
  #define HOLD_MULTIPLIER 0.5
  #define EDITABLE_HOMING_CURRENT
  #define INTERPOLATE true

  #define X_RSENSE     0.11
  #define X_CHAIN_POS  -1
  #define Y_RSENSE     0.11
  #define Y_CHAIN_POS  -1
  #define Z_RSENSE     0.11
  #define Z_CHAIN_POS  -1
  #define Z2_RSENSE    Z_RSENSE
  #define Z2_CHAIN_POS -1
  #define Z3_RSENSE    Z_RSENSE
  #define Z3_CHAIN_POS -1
  #define Z4_RSENSE    Z_RSENSE
  #define Z4_CHAIN_POS -1
  #define E0_RSENSE    0.11
  #define E0_CHAIN_POS -1
  #define E1_RSENSE    E0_RSENSE
  #define E1_CHAIN_POS -1
  #define E2_RSENSE    E0_RSENSE
  #define E2_CHAIN_POS -1
  #define E3_RSENSE    E0_RSENSE
  #define E3_CHAIN_POS -1

  #define STEALTHCHOP_XY
  #define STEALTHCHOP_Z
  #define STEALTHCHOP_I
  #define STEALTHCHOP_J
  #define STEALTHCHOP_K
  #define STEALTHCHOP_U
  #define STEALTHCHOP_V
  #define STEALTHCHOP_W
  #define STEALTHCHOP_E
  #define CHOPPER_TIMING CHOPPER_DEFAULT_24V

  #define MONITOR_DRIVER_STATUS
  #if ENABLED(MONITOR_DRIVER_STATUS)
    #define CURRENT_STEP_DOWN 50
    #define REPORT_CURRENT_CHANGE
    #define STOP_ON_ERROR
  #endif

  #define X_HYBRID_THRESHOLD  100
  #define Y_HYBRID_THRESHOLD  100
  #define Z_HYBRID_THRESHOLD  3
  #define Z2_HYBRID_THRESHOLD 3
  #define Z3_HYBRID_THRESHOLD 3
  #define Z4_HYBRID_THRESHOLD 3
  #define E0_HYBRID_THRESHOLD 30
  #define E1_HYBRID_THRESHOLD 30
  #define E2_HYBRID_THRESHOLD 30
  #define E3_HYBRID_THRESHOLD 30

  #if ENABLED(SENSORLESS_HOMING)
    #define IMPROVE_HOMING_RELIABILITY
  #endif

  #define EDGE_STEPPING
  #define TMC_DEBUG
#endif