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
 *   W : Heating bang-bang window, +/- °C from target (0-20).
 *   K : Cooling bang-bang window, +/- °C from target (0-20).
 *
 * Examples:
 *   M104 I1 H T50 ; Set the second temperature module to heat to 50°C.
 *   M104 I1 T60   ; Set the second temperature module to 60°C, still heating.
 *   M104 I0 T4    ; Set the first temperature module to cool to 4°C.
 *   M104 I0 W0.4 K0.25 ; Set the first module's heating and cooling windows.
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

  const bool has_heat_window = parser.seenval('W');
  celsius_float_t heat_window;
  if (has_heat_window) {
    heat_window = parser.value_float();
    if (!(heat_window >= 0 && heat_window <= 20)) {
      SERIAL_ECHOLN("Heating window must be between 0 and 20°C.");
      return;
    }
  }

  const bool has_cool_window = parser.seenval('K');
  celsius_float_t cool_window;
  if (has_cool_window) {
    cool_window = parser.value_float();
    if (!(cool_window >= 0 && cool_window <= 20)) {
      SERIAL_ECHOLN("Cooling window must be between 0 and 20°C.");
      return;
    }
  }

  const bool has_target = parser.seenval('T');
  celsius_float_t temp;
  if (has_target) {
    temp = parser.value_celsius();
  } else if (!has_heat_window && !has_cool_window) {
    SERIAL_ECHOLN("A target temperature or bang-bang window is required.");
    return;
  }

  const bool set_heating = parser.seen('H');
  const bool set_cooling = parser.seen('C');
  if (set_heating && set_cooling) {
    SERIAL_ECHOLN("Only heating OR cooling can be specified.");
    return;
  } else if (set_heating || set_cooling) {
    if (!has_target) {
      SERIAL_ECHOLN("A target temperature is required when setting the module type.");
      return;
    }
    if (!thermalManager.setTypeHotend(set_heating ? hotend_info_t::deviceType::heater : hotend_info_t::deviceType::cooler, index)) {
      return;
    }
  }

  if (has_heat_window)
    thermalManager.temp_hotend[index].heat_window = heat_window;
  if (has_cool_window)
    thermalManager.temp_hotend[index].cool_window = cool_window;

  if (!has_target) return;

  if (thermalManager.temp_hotend[index].enabled()) {
    thermalManager.setTargetHotend(temp, index);
  } else {
    SERIAL_ECHOLN("Module ", index, " is not enabled. Use H or C to set it to heating or cooling after ensuring the module is wired in the correct orientation.");
  }
}

#endif // HAS_HOTEND
