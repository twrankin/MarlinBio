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
#pragma once

#include <stdint.h>

typedef uint32_t millis_t;

#define SEC_TO_MS(N) millis_t((N)*1000UL)
#define MIN_TO_MS(N) SEC_TO_MS((N)*60UL)
#define MS_TO_SEC(N) millis_t((N)/1000UL)
#define MS_TO_SEC_PRECISE(N) (float(N)/1000.0f)

constexpr bool _PENDING(const millis_t now, const millis_t when) { return int32_t(when - now) > 0; }
constexpr bool _PENDING(const millis_t now, const millis_t start, const millis_t interval) { return (now - start) < interval; }
#define PENDING(V...)  _PENDING(V)
#define ELAPSED(V...) !_PENDING(V)
