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

#if ENABLED(MIXING_EXTRUDER)

#include "../../gcode.h"
#include "../../../feature/mixing.h"

/**
 * MarlinBio:
 * M163: Set mixing extruder ratios.
 * 
 * This command sets the ratios for mixing extruders. The ratio simply scales
 * extrusion; for example, a specified extrusion of 3mm for an extruder with a ratio of
 * 0.5 will extrude 1.5mm. If no parameters are provided, it reports the current settings.
 * 
 *  E : Extruder index.
 *  R : Ratio.
 *
 * Examples:
 *   M163          ; Report the current parameters.
 *   M163 E2 R0.66 ; Set the ratio for the third extruder to 0.66.
 */
void GcodeSuite::M163() {
  if (!parser.seen_any()) {
    /// MarlinBio: Report the current parameters and return.
    M163_report();
    return;
  }

  if (!parser.seenval('E')) {
    SERIAL_ECHOLN("The extruder index, E, is required");
    return;
  }
  const uint8_t extruder = parser.intval('E');
  if (extruder >= EXTRUDERS) {
    SERIAL_ECHOLN("Invalid extruder index");
    return;
  }

  if (!parser.seenval('R')) {
    SERIAL_ECHOLN("The mix ratio, R, is required");
    return;
  }
  const float mix_ratio = parser.value_float();

  bool found = false;
  for (uint8_t i = 0;  i < mixer.mix_config.size(); i++) {
    /// MarlinBio: Don't allow setting ratios for individual extruders.
    if (mixer.mix_config[i].size() > 1) {
      for (uint8_t j = 0; j < mixer.mix_config[i].size(); j++) {
        if (mixer.mix_config[i][j] == extruder) {
          found = true;
          mixer.mix_ratios[i][j] = mix_ratio;
        }
      }
    }
  }

  if (!found) {
    SERIAL_ECHOLN("Invalid extruder index");
    return;
  }
}

void GcodeSuite::M163_report(const bool forReplay/*=true*/) {
  auto header = [](const bool forReplay) {
    report_echo_start(forReplay);
    if (!forReplay) SERIAL_ECHOPGM("  ");
  };

  report_heading(forReplay, FPSTR("M163 - Mixing extruder parameters"));

  header(forReplay);
  SERIAL_ECHO("  Mixing configuration: [");
  for (uint8_t i = 0;  i < mixer.mix_config.size(); i++) {
    SERIAL_ECHO("[");
    for (uint8_t j = 0; j < mixer.mix_config[i].size(); j++) {
      SERIAL_ECHO(mixer.mix_config[i][j]);
      if (j < mixer.mix_config[i].size() - 1) SERIAL_ECHOPGM(", ");
    }
    SERIAL_ECHO("]");
  }
  SERIAL_ECHOLN("]");

  header(forReplay);
  SERIAL_ECHO("  Mix ratios:           [");
  for (uint8_t i = 0;  i < mixer.mix_ratios.size(); i++) {
    SERIAL_ECHO("[");
    for (uint8_t j = 0; j < mixer.mix_ratios[i].size(); j++) {
      SERIAL_ECHO(mixer.mix_ratios[i][j]);
      if (j < mixer.mix_ratios[i].size() - 1) SERIAL_ECHOPGM(", ");
    }
    SERIAL_ECHO("]");
  }
  SERIAL_ECHOLN("]");
}

#endif // MIXING_EXTRUDER
