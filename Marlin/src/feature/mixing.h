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

  /// MarlinBio: Runtime tool count (may be less than EXTRUDERS when extruders are linked).
  FORCE_INLINE static uint8_t get_tool_count() { return mix_config.size(); }

  /// MarlinBio: Find which tool index owns a given physical extruder.
  /// Returns EXTRUDERS if not found.
  static uint8_t tool_for_extruder(uint8_t extruder);

  /// MarlinBio: Link sequential extruders first_ext..last_ext into one tool.
  /// Returns true on success, false on validation failure.
  static bool link(uint8_t first_ext, uint8_t last_ext);

  /// MarlinBio: Unlink a tool back into individual extruders.
  /// Returns true on success, false on validation failure.
  static bool unlink(uint8_t tool);
};

extern Mixer mixer;
