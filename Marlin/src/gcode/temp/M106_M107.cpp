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

#include "../../inc/MarlinConfig.h"

#if HAS_FAN

#include "../gcode.h"
#include "../../module/motion.h"
#include "../../module/temperature.h"

/*
 * MarlinBio:
 * M106: Set PWM.
 * 
 * This command sets the duty cycle for a pulse width modulation header. This
 * is typically used for fans, but can also be used for other devices that accept
 * PWM, such as LEDs. Any command that changes the PWM will disable the auto fan
 * functionality for that header until reenabled.
 * If no parameters are specified, it reports the current settings.
 *
 * Parameters:
 *   I<index> : Header index selector.
 *   P<PWM>   : Target PWM (0 - 255).
 *   A<auto>  : Enable auto fan mode (default: enabled).
 *
 * Examples:
 *   M106         ; Report current parameters
 *   M106 I1 P255 ; Set the second module to full PWM.
 *   M106 I3 P0   ; Turn the fourth module off.
 */
void GcodeSuite::M106() {
  if (!parser.seen_any()) {
    /// MarlinBio: Report the current parameters and return.
    M106_report();
    return;
  }

  const uint8_t index = parser.byteval('I', UINT8_MAX);
  if (index >= FAN_COUNT) {
    SERIAL_ECHOLN("Invalid PWM header index");
    return;
  }

  const uint16_t pwm = parser.ushortval('P', UINT16_MAX);
  if (parser.seen_test('P') && !WITHIN(pwm, 0, FAN_MAX_PWM)) {
    SERIAL_ECHOLN("Invalid PWM");
    return;
  }

  if (parser.seen_test('A')) {
    if (parser.seen_test('P')) {
      SERIAL_ECHOLN("P and A cannot both be specified");
      return;
    }
    thermalManager.auto_fan[index] = Temperature::AutoFan::enabled;
    return;
  }

  /// MarlinBio: This header is being manually controlled, disable auto fan.
  thermalManager.auto_fan[index] = Temperature::AutoFan::disabled;
  thermalManager.set_fan_speed(index, pwm);
}

void GcodeSuite::M106_report(const bool forReplay/*=true*/) {
  auto header = [](const bool forReplay) {
    report_echo_start(forReplay);
    if (!forReplay) SERIAL_ECHOPGM("  ");
  };

  report_heading(forReplay, FPSTR("M106 - Set PWM"));

  header(forReplay);
  SERIAL_ECHO("  Auto fan: [");
  FANS_LOOP(f) {
    SERIAL_ECHO(thermalManager.auto_fan[f] == Temperature::AutoFan::enabled ? "On" : "Off");
    if (f < FAN_COUNT - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");

  header(forReplay);
  SERIAL_ECHO("  PWM:      [");
  FANS_LOOP(f) {
    SERIAL_ECHO(thermalManager.fan_speed[f]);
    if (f < FAN_COUNT - 1) SERIAL_ECHOPGM(", ");
  }
  SERIAL_ECHOLN("]");
}

#endif // HAS_FAN
