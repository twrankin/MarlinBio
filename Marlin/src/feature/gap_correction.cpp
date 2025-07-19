/*
 * MarlinBio 3D Printer Firmware
 * Copyright (c) 2025 MarlinBio [https://github.com/twrankin/MarlinBio]
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
 *
 */

#include "../inc/MarlinConfig.h"

#if HAS_GAP_CORRECTION

#include "gap_correction.h"
#include "../MarlinCore.h"
#include "../module/planner.h"
#include "../module/motion.h"
#include "FDC2214.h"

GapCorrection gapCorrection;

float GapCorrection::capacitance_initial[GC_CHANNEL_NUM] = {0};
float GapCorrection::capacitance_current[GC_CHANNEL_NUM] = {0};

uint8_t GapCorrection::variance_checks[GC_CHANNEL_NUM]      = {0};
float   GapCorrection::capacitance_variance[GC_CHANNEL_NUM] = {0};

/// MarlinBio: Set in settings.reset.
float GapCorrection::touch_threshold_factor;
float GapCorrection::feedrate;
#if GC_DEBUG
  uint16_t GapCorrection::stream_secs;
  millis_t GapCorrection::last_report = 0;
#endif

FDC2214 GapCorrection::sensor = FDC2214();

void GapCorrection::init() {
  sensor.init();
}

void GapCorrection::calibrate() {
  /// MarlinBio: Read the initial capacitances to calibrate the sensor.
  update_capacitances(true);
  for (int i = 0; i < GC_CHANNEL_NUM; i++) {
    capacitance_initial[i]  = capacitance_current[i];
  }
}

/// MarlinBio: The maximum sensible value that could be returned by the FDC2214 is 250nf.
#define CAP_SENSOR_MAX_VALUE 250

void GapCorrection::update_capacitances(bool verify/*=false*/) {
  bool failed = false;

  for (int i = 0; i < GC_CHANNEL_NUM; i++) {
    bool tempFail = false;
    float accum   = 0;
    /// MarlinBio: Read the value GC_SENSOR_READS times and average.
    for (int j = 0; j < GC_SENSOR_READS; j++) {
      /// MarlinBio: TODO: Possibly remove outliers
      accum += sensor.read_sensor(i);
    }
    capacitance_current[i] = accum / GC_SENSOR_READS;

    /// MarlinBio: Check for bad values, and make sure the value is changing at least a tiny bit.
    if (capacitance_current[i] <= 0) tempFail = true;
    if (capacitance_current[i] > CAP_SENSOR_MAX_VALUE) tempFail = true;
    if (capacitance_variance[i] != capacitance_current[i]) variance_checks[i] = 0;
    if (++variance_checks[i] >= GC_VARIANCE_CHECK_LIMIT) tempFail = true;
    capacitance_variance[i] = capacitance_current[i];

    if (tempFail) SERIAL_ECHOLN("Channel ", i, " bad value: ", capacitance_current[i]);
    failed = failed || tempFail;
  }

  #if GC_DEBUG
    if (stream_secs > 0) {
      print_capacitances();
    }
  #endif

  if (failed && verify) {
    kill(F("Gap Correction: Capacitance sensor bad value"));
  }
}

/// MarlinBio: This conversion has been experimentally determined.
/// To allow for more dynamic situations, the formula may need to
/// be modifiable by the G789 command in the future.
float GapCorrection::capacitance_to_mm(float capacitance) {
  return MIN(GC_MAX_TRAVEL, (30 - capacitance) / 3);
}

void GapCorrection::branch_point() {
  /// MarlinBio: Set the feedrate for our upcoming moves; we restore it before returning.
  feedRate_t tempFR = feedrate_mm_s;
  feedrate_mm_s = feedrate;

  uint8_t attempts = GC_ATTEMPT_LIMIT;
  while (--attempts) {
    update_capacitances(true);

    /// MarlinBio: Determine if the measured capacitances pass the threshold for touch detection
    /// and which direction has the strongest signal (by percentage increase over initial).
    int8_t x_dir = 0, y_dir = 0;
    float x_plus_change  = (x_plus_capacitance()  - x_plus_initial())  / x_plus_initial();
    float x_minus_change = (x_minus_capacitance() - x_minus_initial()) / x_minus_initial();
    float y_plus_change  = (y_plus_capacitance()  - y_plus_initial())  / y_plus_initial();
    float y_minus_change = (y_minus_capacitance() - y_minus_initial()) / y_minus_initial();
    float x_cap_cur = 0, x_cap_init = 0, y_cap_cur = 0, y_cap_init = 0;
    /// MarlinBio: Using algebra, cur / init >= factor == (cur - init) / init >= factor - 1.
    float th = touch_threshold_factor - 1;
    if (x_plus_change >= th || x_minus_change >= th || y_plus_change >= th || y_minus_change >= th) {
      /// MarlinBio: We've successfully connected the lines.
      break;
    }
    if (x_plus_change > 0 && x_plus_change > x_minus_change) {
      x_dir      = 1;
      x_cap_cur  = x_plus_capacitance();
      x_cap_init = x_plus_initial();
    } else if (x_minus_change > 0) {
      x_dir      = -1;
      x_cap_cur  = x_minus_capacitance();
      x_cap_init = x_minus_initial();
    }
    if (y_plus_change > 0 && y_plus_change > y_minus_change) {
      y_dir      = 1;
      y_cap_cur  = y_plus_capacitance();
      y_cap_init = y_plus_initial();
    } else if (y_minus_change > 0) {
      y_dir      = -1;
      y_cap_cur  = y_minus_capacitance();
      y_cap_init = y_minus_initial();
    }

    if (x_dir == 0 && y_dir == 0) {
      /// MarlinBio: If all readings were less than initial, try to read the sensors again.
      continue;
    }

    destination = current_position;

    float dx = capacitance_to_mm(x_cap_cur) * x_dir, dy = capacitance_to_mm(y_cap_cur) * y_dir;
    destination.x += dx;
    destination.y += dy;
    if (TERN1(HAS_CONSTANT_EXTRUSION, !planner.constant_extrusion_enabled)) {
      /// MarlinBio: Calculate extrusion as follows: e = (needle_diam/syringe_diam)^2 * sqrt(dx^2 + dy^2).
      float t1 = (planner.needle_inner_diameter[active_extruder] / planner.syringe_inner_diameter[active_extruder]);
      float t2 = HYPOT(dx, dy);
      destination.e += t1 * t1 * t2;
    }
    prepare_line_to_destination(true);
  }

  if (attempts == 0) {
    kill(F("Gap Correction: A connection was never detected."));
  }

  feedrate_mm_s = tempFR;
}

void GapCorrection::print_capacitances() {
  SERIAL_ECHO("  Current capacitances:");
  SERIAL_ECHO(" X+:", gapCorrection.x_plus_capacitance());
  SERIAL_ECHO(" X-:", gapCorrection.x_minus_capacitance());
  SERIAL_ECHO(" Y+:", gapCorrection.y_plus_capacitance());
  SERIAL_ECHOLN(" Y-:", gapCorrection.y_minus_capacitance());
}

#if GC_DEBUG
  void GapCorrection::debug_stream(millis_t time/*=millis()*/) {
    if (stream_secs > 0 && ELAPSED(time, last_report, SEC_TO_MS(stream_secs))) {
      last_report = time;
      update_capacitances();
    }
  }
#endif

#endif // HAS_GAP_CORRECTION
