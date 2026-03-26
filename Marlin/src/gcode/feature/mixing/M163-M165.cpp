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

#if ENABLED(MIXING_EXTRUDER)

#include "../../gcode.h"
#include "../../../feature/mixing.h"
#include "../../../module/motion.h"

/**
 * MarlinBio:
 * M163: Set mixing extruder ratios.
 * 
 * This command sets the ratios for mixing extruders. The ratio simply scales
 * extrusion; for example, a specified extrusion of 3mm for an extruder with a ratio of
 * 0.5 will extrude 1.5mm. If no parameters are provided, it reports the current settings.
 * 
 * Parameters:
 *   E : Extruder index.
 *   R : Ratio.
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

/**
 * MarlinBio:
 * M164: Link or unlink mixing extruders at runtime.
 *
 * When linking, the new tool's offset is computed as the midpoint of
 * the linked extruders' original offsets. When unlinking, offsets revert
 * to the per-extruder defaults from Configuration.h.
 *
 * Parameters:
 *   L<first> U<last> : Link extruders first..last into one tool.
 *   U<tool>          : Unlink tool back to individual extruders (only when L is absent).
 *   (no params)      : Report current tool mapping.
 *
 * Examples:
 *   M164 L1 U2    ; Link extruders 1 and 2 into one tool.
 *   M164 L0 U2    ; Link extruders 0, 1, and 2 into one tool.
 *   M164 U0       ; Unlink tool 0 back to individual extruders.
 *   M164          ; Report current configuration.
 */
void GcodeSuite::M164() {
  if (!parser.seen_any()) {
    M164_report();
    return;
  }

  if (parser.seen('L')) {
    // Link operation: L = first extruder, U = last extruder
    const uint8_t first_ext = parser.byteval('L');
    if (!parser.seenval('U')) {
      SERIAL_ECHOLN("U (last extruder index) is required for linking");
      return;
    }
    const uint8_t last_ext = parser.byteval('U');
    if (!mixer.link(first_ext, last_ext)) {
      SERIAL_ECHOLN("Link failed: invalid range or extruders already in a group");
      return;
    }
    SERIAL_ECHOLN("Linked extruders ", first_ext, " through ", last_ext);
  }
  else if (parser.seen('U')) {
    // Unlink operation: U = tool index
    const uint8_t tool = parser.byteval('U');
    if (!mixer.unlink(tool)) {
      SERIAL_ECHOLN("Unlink failed: tool index invalid or not a linked group");
      return;
    }
    SERIAL_ECHOLN("Unlinked tool ", tool);
  }

  M164_report();
}

void GcodeSuite::M164_report(const bool forReplay/*=true*/) {
  report_heading(forReplay, FPSTR("M164 - Tool/extruder mapping"));

  for (uint8_t t = 0; t < mixer.mix_config.size(); t++) {
    report_echo_start(forReplay);
    SERIAL_ECHOPGM("  T"); SERIAL_ECHO(t);
    SERIAL_ECHOPGM(": Ext {");
    const auto &group = mixer.mix_config[t];
    for (uint8_t j = 0; j < group.size(); j++) {
      SERIAL_ECHO(group[j]);
      if (j < group.size() - 1) SERIAL_ECHOPGM(", ");
    }
    SERIAL_ECHOPGM("}");
    #if HAS_HOTEND_OFFSET
      SERIAL_ECHOPGM("  Offset X:", hotend_offset[t].x,
                             " Y:", hotend_offset[t].y,
                             " Z:", hotend_offset[t].z);
    #endif
    SERIAL_EOL();
  }
}

#endif // MIXING_EXTRUDER
