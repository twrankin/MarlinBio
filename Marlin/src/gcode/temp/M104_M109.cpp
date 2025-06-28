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

#include "../../inc/MarlinConfigPre.h"

#if HAS_HOTEND

#include "../gcode.h"
#include "../../module/temperature.h"

/**
 * MarlinBio:
 * M104: Set temperature module target.
 *
 * Parameters
 *  T<index>  : Temperature module index.
 *  S<target> : The target temperature in celsius.
 *
 * Examples
 *  M104 T1 S60 : Set the second temperature module to 60°.
 */
void GcodeSuite::M104_M109(const bool isM109) {
  /// MarlinBio: A lot of the original fluff was removed. It simply sets the temperature
  /// for the specified module now.
  /// Leaving the parameters as T and S to avoid differences with the Marlin documentation.
  /// Though E(xtruder) or H(otend) and T(emperature) are obviously better choices.
  const uint8_t module = parser.byteval('T', UINT8_MAX);
  if (module >= HOTENDS) {
    SERIAL_ECHOLN("Invalid temperature module index");
    return;
  }

  celsius_float_t temp;
  if (parser.seenval('S')) {
    temp = parser.value_celsius();
  } else {
    SERIAL_ECHOLN("A temperature is required");
    return;
  }

  thermalManager.setTargetHotend(temp, module);
}

#endif // HAS_HOTEND
