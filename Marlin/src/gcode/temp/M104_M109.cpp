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

#include "../../inc/MarlinConfigPre.h"

#if HAS_HOTEND

#include "../gcode.h"
#include "../../module/temperature.h"

/*
 * MarlinBio:
 * M104: Set temperature module type and target.
 * 
 * This command can set a temperature module to heat or cool to a target
 * temperature, or disable it. Using D without specifying a module will
 * disable all modules. A module can only go from disabled to heating or
 * cooling, or from heating or cooling to disabled. 
 *
 * Parameters:
 *   I : Module index selector.
 *   T : Target temperature (°C).
 *   D : Disable. If no index is specified, disable all.
 *   H : Specify heating.
 *   C : Specify cooling.
 *
 * Examples:
 *   M104 I1 H T50 ; Set the second temperature module to heat to 50°C.
 *   M104 I1 T60   ; Set the second temperature module to 60°C, still heating.
 *   M104 I0 T4    ; Set the first temperature module to cool to 4°C.
 *   M104 I0 D     ; Disable the first temperature module.
 */
void GcodeSuite::M104_M109(const bool isM109) {
  bool disable = parser.seen('D');
  bool indexed = parser.seenval('I');

  if (!indexed && !disable) {
    SERIAL_ECHOLN("A module index is required, except when disabling everything using D.");
    return;
  }

  uint8_t index;
  if (indexed) {
    index = parser.value_byte();
    if (index >= HOTENDS) {
      SERIAL_ECHOLN("Invalid temperature module index.");
      return;
    }
  }

  if (disable) {
    if (indexed) {
      thermalManager.disable_heater(index);
    } else {
      thermalManager.disable_all_heaters();
    }

    /// MarlinBio: When D is specified, do nothing else.
    return;
  }

  celsius_float_t temp;
  if (parser.seenval('T')) {
    temp = parser.value_celsius();
  } else {
    SERIAL_ECHOLN("A target temperature is required.");
    return;
  }

  if (parser.seen('H') && parser.seen('C')) {
    SERIAL_ECHOLN("Only heating OR cooling can be specified.");
    return;
  } else if (parser.seen('H') || parser.seen('C')) {
    if (!thermalManager.setTypeHotend(parser.seen('H') ? hotend_info_t::deviceType::heater : hotend_info_t::deviceType::cooler, index)) {
      return;
    }
  }

  if (thermalManager.temp_hotend[index].enabled()) {
    thermalManager.setTargetHotend(temp, index);
  } else {
    SERIAL_ECHOLN("Module ", index, " is not enabled. Use H or C to set it to heating or cooling after ensuring the module is wired in the correct orientation.");
  }
}

#endif // HAS_HOTEND
