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

 #include "../../../inc/MarlinConfig.h"

 #if ENABLED(SWITCHING_NOZZLE)

 #include "../../gcode.h"
 #include "../../../module/motion.h"


 void GcodeSuite::M170(){
    if (!parser.seen_any()) {
    /// MarlinBio: Report the current parameters and return.
    M170_report();
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

    /*
      Going to need to finish creating the janus".cpp" and "janus.h".
      When that's done replace all instances of mixer with janus
    */

    /// MarlinBio: Find which tool owns this extruder and update its ratio.
    const uint8_t tool = mixer.tool_for_extruder(extruder);
    if (tool >= EXTRUDERS || mixer.mix_config[tool].size() <= 1) {
      SERIAL_ECHOLN("Extruder is not part of a linked group");
      return;
    }
    // Find the index within the group
    const auto &group = mixer.mix_config[tool];
    for (uint8_t j = 0; j < group.size(); j++) {
      if (group[j] == extruder) {
        mixer.mix_ratios[tool][j] = mix_ratio;
        return;
      }
    }
 }

void GcodeSuite::M170_report(const bool forReplay/*=true*/) {
  auto header = [](const bool forReplay) {
    report_echo_start(forReplay);
    if (!forReplay) SERIAL_ECHOPGM("  ");
  };

   report_heading(forReplay, FPSTR("M170 - Switching extruder parameters"));

 }