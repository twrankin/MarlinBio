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

#include "../inc/MarlinConfig.h"

#if ENABLED(MIXING_EXTRUDER)

#include "mixing.h"
#include "../module/stepper.h"

Mixer mixer;

/// MarlinBio: Initialized in settings.reset.
std::vector<std::vector<uint8_t>> Mixer::mix_config;
std::vector<std::vector<float>>   Mixer::mix_ratios;

/// MarlinBio: Adjust the Z locks so the extruder systems that are linked move in tandem.
void Mixer::update_locks(const uint_fast8_t tool) {
  #if ENABLED(Z_MULTI_ENDSTOPS)
    stepper.set_all_z_lock(true);
    MIXER_STEPPER_LOOP(i, tool) {
      switch(i) {
      case 0: stepper.set_z1_lock(false); break;
      case 1: stepper.set_z2_lock(false); break;
      #if NUM_Z_STEPPERS > 2
        case 2: stepper.set_z3_lock(false); break;
        #if NUM_Z_STEPPERS > 3
          case 3: stepper.set_z4_lock(false); break;
        #endif
      #endif
      }
    }
  #endif
}

void Mixer::init() {
  update_locks(active_extruder);
}

void Mixer::T(const uint_fast8_t new_tool) {
  update_locks(new_tool);
}

#endif // MIXING_EXTRUDER
