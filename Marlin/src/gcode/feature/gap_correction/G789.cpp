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

#include "../../../inc/MarlinConfig.h"
#include "../../gcode.h"
#include "../../../module/planner.h"
#include "../../../feature/gap_correction.h"

#if HAS_GAP_CORRECTION

/**
 * MarlinBio:
 * G789: Set gap correction parameters and initiate branch point handling.
 * 
 * This command can set the parameters for gap correction, or initiate some actions.
 * If no parameters are provided, it reports the current settings.
 *
 * Parameters: 
 *   C : Calibrate by reading the current capacitances to set the initial values and return.
 *   U : Update the capacitances by reading current values and return. NOTE: This is only for
 *       informational purposes, branch point handling will update the capacitances as needed.
 *   T : The touch threshold factor. See GC_TOUCH_THRESHOLD_FACTOR in Configuration.h.
 *   F : The feedrate for gap correction moves. See GC_FEEDRATE in Configuration.h.
 *   B : Initiate branch point handling.
 *   S : Start debug streaming, reading and reporting the capacitances every interval (ms).
 */
void GcodeSuite::G789() {
  if (!parser.seen_any()) {
    /// MarlinBio: Report the current parameters and return.
    G789_report();
    return;
  }

  if (parser.seen('C')) {
    /// MarlinBio: Calibrating the sensor should be done initially inside
    /// the support bath before printing begins.
    gapCorrection.calibrate();
    return;
  }

  if (parser.seen('U')) {
    /// MarlinBio: For informational purposes.
    /// The branch point logic updates capacitances as needed.
    gapCorrection.update_capacitances();
    return;
  }

  if (parser.seenval('T')) {
    float temp = parser.value_float();
    if (temp > 1) {
      gapCorrection.touch_threshold_factor = temp;
    } else {
      SERIAL_ECHOLN("Invalid threshold factor");
      return;
    }
  }

  if (parser.seenval('F')) {
    float temp = MMM_TO_MMS(parser.value_linear_units());
    if (temp > 0 && temp <= planner.settings.max_feedrate_mm_s[E_AXIS_N(active_extruder)]) {
      gapCorrection.feedrate = temp;
    } else {
      SERIAL_ECHOLN("Invalid feedrate");
      return;
    }
  }

  #if GC_DEBUG
    /// MarlinBio: Continually stream sensor readings.
    if (parser.seenval('S')) {
      float val = parser.value_float();
      if (val > 0) {
        gapCorrection.stream_ms = SEC_TO_MS(val);
      } else {
        SERIAL_ECHOLN("Invalid streaming interval");
      }
    }
  #endif

  if (parser.seen('B')) {
    gapCorrection.branch_point();
    return;
  }
}

void GcodeSuite::G789_report(const bool forReplay/*=true*/) {
  auto header = [](const bool forReplay) {
    report_echo_start(forReplay);
    if (!forReplay) SERIAL_ECHOPGM("  ");
  };

  report_heading(forReplay, FPSTR("G789 - Gap correction"));

  header(forReplay);
  SERIAL_ECHO("  Threshold factor:", gapCorrection.touch_threshold_factor);
  SERIAL_ECHOLN(", Feedrate:", uint(MMS_TO_MMM(gapCorrection.feedrate)));

  header(forReplay);
  SERIAL_ECHO("  Initial capacitances:");
  SERIAL_ECHO(" X+:", gapCorrection.x_plus_initial());
  SERIAL_ECHO(" X-:", gapCorrection.x_minus_initial());
  SERIAL_ECHO(" Y+:", gapCorrection.y_plus_initial());
  SERIAL_ECHOLN(" Y-:", gapCorrection.y_minus_initial());

  header(forReplay);
  gapCorrection.print_capacitances();
}

#endif // HAS_GAP_CORRECTION
