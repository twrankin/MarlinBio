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

#pragma once

#include "../inc/MarlinConfig.h"

#define MIXER_STEPPER_LOOP(VAR, tool) for (auto VAR : mixer.get_steppers(tool))

/**
 * MarlinBio:
 * @brief Mixer class
 * @details Contains data and logic for mixing extruders.
 *          This has been completely reworked from the original
 *          MarlinFirmware implementation to better support our needs.
 */
class Mixer {

  static void update_locks(const uint_fast8_t tool);

  public:

  static std::vector<std::vector<uint8_t>> mix_config;
  static std::vector<std::vector<float>>   mix_ratios;

  static void init();

  static void T(const uint_fast8_t new_tool);

  FORCE_INLINE static const std::vector<uint8_t> get_steppers(uint8_t tool) {
    return mix_config[tool];
  }
  FORCE_INLINE static const std::vector<float> get_ratios(uint8_t tool) {
    return mix_ratios[tool];
  }
};

extern Mixer mixer;
