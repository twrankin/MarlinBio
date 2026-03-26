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
#include "../module/planner.h"
#include "../module/motion.h"
#include "../module/tool_change.h"

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

uint8_t Mixer::tool_for_extruder(uint8_t extruder) {
  for (uint8_t t = 0; t < mix_config.size(); t++)
    for (auto e : mix_config[t])
      if (e == extruder) return t;
  return EXTRUDERS;
}

bool Mixer::link(uint8_t first_ext, uint8_t last_ext) {
  // Validate range
  if (first_ext >= last_ext || last_ext >= EXTRUDERS) return false;

  // Find which tools currently contain extruders first_ext through last_ext.
  // All must be individual (size == 1) and sequential in the tool list.
  int first_tool = -1, last_tool = -1;
  for (uint8_t ext = first_ext; ext <= last_ext; ext++) {
    const uint8_t t = tool_for_extruder(ext);
    if (t >= EXTRUDERS) return false;               // Extruder not found
    if (mix_config[t].size() != 1) return false;    // Already part of a group
    if (first_tool < 0) first_tool = t;
    last_tool = t;
  }

  // The tools must be contiguous in the tool list
  if (last_tool - first_tool != last_ext - first_ext) return false;

  // Drain queued moves before modifying config
  planner.synchronize();

  // Build the new merged group
  std::vector<uint8_t> new_group;
  std::vector<float>   new_ratios;
  for (uint8_t ext = first_ext; ext <= last_ext; ext++) {
    new_group.push_back(ext);
    new_ratios.push_back(1.0f);
  }

  // Save z_pos for the merged tool (use first extruder's z_pos)
  const float merged_z = toolchange_settings.z_pos[first_tool];

  // Replace first tool entry with the merged group, erase the rest
  mix_config[first_tool] = new_group;
  mix_ratios[first_tool] = new_ratios;

  const uint8_t tools_removed = last_tool - first_tool;
  mix_config.erase(mix_config.begin() + first_tool + 1, mix_config.begin() + last_tool + 1);
  mix_ratios.erase(mix_ratios.begin() + first_tool + 1, mix_ratios.begin() + last_tool + 1);

  // Shift z_pos array to match new compact layout
  toolchange_settings.z_pos[first_tool] = merged_z;
  for (uint8_t t = first_tool + 1; t < (uint8_t)mix_config.size(); t++)
    toolchange_settings.z_pos[t] = toolchange_settings.z_pos[t + tools_removed];
  for (uint8_t t = mix_config.size(); t < EXTRUDERS; t++)
    toolchange_settings.z_pos[t] = Z_HOME_POS;

  // Fix active_extruder if it pointed to a tool that shifted
  if (active_extruder > first_tool && active_extruder <= (uint8_t)last_tool)
    active_extruder = first_tool;
  else if (active_extruder > (uint8_t)last_tool)
    active_extruder -= tools_removed;

  // Recompute tool offsets from default per-extruder offsets
  TERN_(HAS_HOTEND_OFFSET, recompute_hotend_offsets());

  update_locks(active_extruder);
  return true;
}

bool Mixer::unlink(uint8_t tool) {
  if (tool >= mix_config.size()) return false;
  if (mix_config[tool].size() <= 1) return false;   // Not a linked group

  // Drain queued moves before modifying config
  planner.synchronize();

  const auto members = mix_config[tool]; // copy before modifying
  const uint8_t new_tools = members.size() - 1; // how many extra tools we're adding

  // Replace the group entry with individual entries
  mix_config[tool] = { members[0] };
  mix_ratios[tool] = { 1.0f };
  for (uint8_t i = 1; i < members.size(); i++) {
    mix_config.insert(mix_config.begin() + tool + i, std::vector<uint8_t>{ members[i] });
    mix_ratios.insert(mix_ratios.begin() + tool + i, std::vector<float>{ 1.0f });
  }

  // Shift z_pos array: keep first extruder's z_pos, set Z_HOME_POS for newly split ones
  // First, shift existing entries after the tool to make room
  for (int t = (int)mix_config.size() - 1; t >= (int)(tool + members.size()); t--)
    toolchange_settings.z_pos[t] = toolchange_settings.z_pos[t - new_tools];
  // Set new individual z_pos values
  for (uint8_t i = 1; i < members.size(); i++)
    toolchange_settings.z_pos[tool + i] = Z_HOME_POS;

  // active_extruder stays at same index (was group head, now first individual).
  // But if active was beyond the unlinked tool, shift it up.
  if (active_extruder > tool)
    active_extruder += new_tools;

  // Recompute tool offsets — reverts to default per-extruder offsets
  TERN_(HAS_HOTEND_OFFSET, recompute_hotend_offsets());

  update_locks(active_extruder);
  return true;
}

#endif // MIXING_EXTRUDER
