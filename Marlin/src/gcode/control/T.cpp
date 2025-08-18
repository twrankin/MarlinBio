/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
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
 *
 */

#include "../../inc/MarlinConfigPre.h"

#if HAS_TOOLCHANGE

#include "../gcode.h"
#include "../../module/tool_change.h"

#include "../../module/motion.h"

/*
 * MarlinBio:
 * T: Switch tool.
 * 
 * Switch between tools to indicate which motors Z and E control.
 * Without mixing, tools correspond 1:1 with extruders. With mixing,
 * linked extruders are considered one tool. It is not recommended to use
 * the S1 parameter, as this can disrupt tracking. If no movement after tool
 * change is desired, change the associated setting instead.
 *
 * Parameters:
 *   T : The new tool index.
 *   S : Allow (0) or block (1) movement for the tool change, default 0.
 * 
 * Examples:
 *   T1 ; Switch to the second tool.
 */
void GcodeSuite::T(const int8_t tool_index) {

  #if HAS_MULTI_EXTRUDER
    // For 'T' with no parameter report the current tool.
    if (parser.string_arg && *parser.string_arg == '*') {
      SERIAL_ECHOLNPGM(STR_ACTIVE_TOOL, active_extruder);
      return;
    }
  #endif

  // Count this command as movement / activity
  reset_stepper_timeout();

  tool_change(tool_index, parser.boolval('S') || tool_index == active_extruder);
}

#endif // HAS_TOOLCHANGE
