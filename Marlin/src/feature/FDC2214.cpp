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

#include "../inc/MarlinConfig.h"

#if NEED_FDC2214

#include "FDC2214.h"

static float caps[_GC_CHANNEL_NUM] = {21, 4, 12, 9};
static uint8_t reads[_GC_CHANNEL_NUM] = {0};
float FDC2214::read_sensor(uint8_t sensor) {
  float ret = 3;

  if (reads[sensor] >= 5) {
    if (reads[sensor] != 5 && reads[sensor] % GC_SENSOR_READS == 0) {
      if (sensor == GC_X_PLUS_CHANNEL || sensor == GC_Y_PLUS_CHANNEL) {
        caps[sensor] += 0.6;
      } else if (sensor == GC_X_MINUS_CHANNEL || sensor == GC_Y_MINUS_CHANNEL) {
        caps[sensor] -= 0.2;
      }
    }
    ret = caps[sensor];
  }

  reads[sensor]++;
  return ret;
}

#endif // NEED_FDC2214
