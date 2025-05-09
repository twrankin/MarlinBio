/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
#ifndef ENV_VALIDATE_H
#define ENV_VALIDATE_H

#if NOT_TARGET(__STM32F1__, STM32F1)
  #if NONE(ALLOW_STM32F4, ALLOW_GD32F3, ALLOW_GD32F1)
    #error "Oops! Select an STM32F1 board in 'Tools > Board.'"
  #elif ENABLED(ALLOW_STM32F4) && NOT_TARGET(STM32F4)
    #error "Oops! Select an STM32F4 board in 'Tools > Board.'"
  #elif ENABLED(ALLOW_GD32F3) && NOT_TARGET(ARDUINO_ARCH_MFL)
    #error "Oops! Make sure you have a GD32F3 MFL environment selected."
  #elif ENABLED(ALLOW_GD32F1) && NOT_TARGET(ARDUINO_ARCH_MFL)
    #error "Oops! Make sure you have a GD32F1 MFL environment selected."
  #endif
#endif

#undef ALLOW_STM32F4
#undef ALLOW_GD32F3
#undef ALLOW_GD32F1

#endif
