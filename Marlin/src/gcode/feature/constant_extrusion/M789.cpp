/**
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
#include "../../../module/stepper.h"

#if HAS_CONSTANT_EXTRUSION

/**
 * MarlinBio:
 * M789: Set constant extrusion parameters.
 * 
 * This command sets the parameters for constant extrusion.
 * If no parameters are provided, it reports the current settings.
 * If any parameter is provided, it will enable constant extrusion unless 'D' is specified.
 * 
 *  E : Extruder index (default: active_extruder)
 *  S : Syringe inner diameter (mm)
 *  N : Needle inner diameter (mm)
 *  K : Extrusion coefficient
 *  P : Pressurization length (mm)
 *  D : Disable constant extrusion (default: enabled)
 *
 * Examples:
 *   M789                             ; Report current parameters
 *   M789 E0 S12.06 N0.603 K1.8 P0.33 ; Set parameters for the first extruder
 *   M789 D                           ; Disable constant extrusion
 */
void GcodeSuite::M789() {
  if (!parser.seen_any()) {
    /// MarlinBio: Report the current parameters and return.
    M789_report();
    return;
  }

  uint8_t extruder = parser.byteval('E', active_extruder);
  if (extruder >= EXTRUDERS) {
    SERIAL_ECHOLN("Invalid extruder index");
    return;
  }

  gcode.constant_extrusion_enabled         = !parser.seen('D');
  planner.constant_extrusion_enabled       = gcode.constant_extrusion_enabled;
  planner.syringe_inner_diameter[extruder] = parser.floatval('S', planner.syringe_inner_diameter[extruder]);
  planner.needle_inner_diameter[extruder]  = parser.floatval('N', planner.needle_inner_diameter[extruder]);
  planner.extrusion_coefficient[extruder]  = parser.floatval('K', planner.extrusion_coefficient[extruder]);
  planner.pressurization_length[extruder]  = parser.floatval('P', planner.pressurization_length[extruder]);
}

void GcodeSuite::M789_report(const bool forReplay/*=true*/) {
  auto header = [](const bool forReplay) {
    report_echo_start(forReplay);
    if (!forReplay) SERIAL_ECHOPGM("  ");
  };

  report_heading(forReplay, FPSTR("M789 - Constant extrusion parameters"));

  header(forReplay);
  SERIAL_ECHO("Constant extrusion ");
  if (planner.constant_extrusion_enabled)
    SERIAL_ECHOLN("enabled");
  else
    SERIAL_ECHOLN("disabled");

  header(forReplay);
  SERIAL_ECHO("  Syringe Inner Diameter: [");
  EXTRUDER_LOOP() {
    SERIAL_ECHO(planner.syringe_inner_diameter[e]);
    if (e < EXTRUDERS - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");

  header(forReplay);
  SERIAL_ECHO("  Needle Inner Diameter:  [");
  EXTRUDER_LOOP() {
    SERIAL_ECHO(planner.needle_inner_diameter[e]);
    if (e < EXTRUDERS - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");

  header(forReplay);
  SERIAL_ECHO("  Extrusion Coefficient:  [");
  EXTRUDER_LOOP() {
    SERIAL_ECHO(planner.extrusion_coefficient[e]);
    if (e < EXTRUDERS - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");

  header(forReplay);
  SERIAL_ECHO("  Pressurization Length:  [");
  EXTRUDER_LOOP() {
    SERIAL_ECHO(planner.pressurization_length[e]);
    if (e < EXTRUDERS - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");
}

#endif // HAS_CONSTANT_EXTRUSION
