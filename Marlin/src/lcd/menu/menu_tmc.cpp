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

//
// TMC Menu
//

#include "../../inc/MarlinConfigPre.h"

#if HAS_MARLINUI_MENU && HAS_TRINAMIC_CONFIG

#include "menu_item.h"
#include "../../module/stepper/indirection.h"
#include "../../feature/tmc_util.h"

#define TMC_EDIT_STORED_I_RMS(ST,STR) EDIT_ITEM_FAST_F(uint16_4, F(STR), &stepper##ST.val_mA, 100, 3000, []{ stepper##ST.refresh_stepper_current(); })

void menu_tmc_current() {
  START_MENU();
  BACK_ITEM(MSG_TMC_DRIVERS);
  TERN_(X_IS_TRINAMIC,  TMC_EDIT_STORED_I_RMS(X, STR_A));
  TERN_(X2_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(X2, STR_X2));
  TERN_(Y_IS_TRINAMIC,  TMC_EDIT_STORED_I_RMS(Y, STR_B));
  TERN_(Y2_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(Y2, STR_Y2));
  TERN_(Z_IS_TRINAMIC,  TMC_EDIT_STORED_I_RMS(Z, STR_C));
  TERN_(Z2_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(Z2, STR_Z2));
  TERN_(Z3_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(Z3, STR_Z3));
  TERN_(Z4_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(Z4, STR_Z4));
  TERN_(E0_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E0, STR_E0));
  TERN_(E1_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E1, STR_E1));
  TERN_(E2_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E2, STR_E2));
  TERN_(E3_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E3, STR_E3));
  TERN_(E4_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E4, STR_E4));
  TERN_(E5_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E5, STR_E5));
  TERN_(E6_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E6, STR_E6));
  TERN_(E7_IS_TRINAMIC, TMC_EDIT_STORED_I_RMS(E7, STR_E7));
  END_MENU();
}

#if ENABLED(HYBRID_THRESHOLD)

  #define TMC_EDIT_STORED_HYBRID_THRS(ST, STR) EDIT_ITEM_FAST_F(uint16_3, F(STR), &stepper##ST.stored.hybrid_thrs, 0, STEPPER_MAX_THRS(ST), []{ stepper##ST.refresh_hybrid_thrs(); });

  void menu_tmc_hybrid_thrs() {
    START_MENU();
    BACK_ITEM(MSG_TMC_DRIVERS);
    TERN_( X_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(X,  STR_X));
    TERN_(X2_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(X2, STR_X2));
    TERN_( Y_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Y,  STR_Y));
    TERN_(Y2_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Y2, STR_Y2));
    TERN_( Z_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Z,  STR_Z));
    TERN_(Z2_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Z2, STR_Z2));
    TERN_(Z3_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Z3, STR_Z3));
    TERN_(Z4_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(Z4, STR_Z4));
    TERN_(E0_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E0, STR_E0));
    TERN_(E1_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E1, STR_E1));
    TERN_(E2_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E2, STR_E2));
    TERN_(E3_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E3, STR_E3));
    TERN_(E4_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E4, STR_E4));
    TERN_(E5_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E5, STR_E5));
    TERN_(E6_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E6, STR_E6));
    TERN_(E7_HAS_STEALTHCHOP, TMC_EDIT_STORED_HYBRID_THRS(E7, STR_E7));
    END_MENU();
  }

#endif // HYBRID_THRESHOLD

#if ENABLED(SENSORLESS_HOMING)

  #define TMC_EDIT_STORED_SGT(ST) EDIT_ITEM_F(int4, F(STR_##ST), &stepper##ST.stored.homing_thrs, stepper##ST.sgt_min, stepper##ST.sgt_max, []{ stepper##ST.refresh_homing_thrs(); });

  void menu_tmc_homing_thrs() {
    START_MENU();
    BACK_ITEM(MSG_TMC_DRIVERS);
    TERN_( X_SENSORLESS, TMC_EDIT_STORED_SGT(X));
    TERN_(X2_SENSORLESS, TMC_EDIT_STORED_SGT(X2));
    TERN_( Y_SENSORLESS, TMC_EDIT_STORED_SGT(Y));
    TERN_(Y2_SENSORLESS, TMC_EDIT_STORED_SGT(Y2));
    TERN_( Z_SENSORLESS, TMC_EDIT_STORED_SGT(Z));
    TERN_(Z2_SENSORLESS, TMC_EDIT_STORED_SGT(Z2));
    TERN_(Z3_SENSORLESS, TMC_EDIT_STORED_SGT(Z3));
    TERN_(Z4_SENSORLESS, TMC_EDIT_STORED_SGT(Z4));
    TERN_( I_SENSORLESS, TMC_EDIT_STORED_SGT(I));
    TERN_( J_SENSORLESS, TMC_EDIT_STORED_SGT(J));
    TERN_( K_SENSORLESS, TMC_EDIT_STORED_SGT(K));
    TERN_( U_SENSORLESS, TMC_EDIT_STORED_SGT(U));
    TERN_( V_SENSORLESS, TMC_EDIT_STORED_SGT(V));
    TERN_( W_SENSORLESS, TMC_EDIT_STORED_SGT(W));
    END_MENU();
  }

#endif // SENSORLESS_HOMING

#if ENABLED(EDITABLE_HOMING_CURRENT)

  #define TMC_EDIT_HOMING_CURRENT(ST, STR) EDIT_ITEM_FAST_F(uint16_4, F(STR), &homing_current_mA.ST, ST##_CURRENT / 3, ST##_CURRENT)

  void menu_tmc_homing_current() {
    START_MENU();
    STATIC_ITEM(MSG_TMC_HOMING_CURRENT);
    BACK_ITEM(MSG_TMC_DRIVERS);
    TERN_( X_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(X,  STR_X));
    TERN_(X2_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(X2, STR_X2));
    TERN_( Y_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Y,  STR_Y));
    TERN_(Y2_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Y2, STR_Y2));
    TERN_( Z_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Z,  STR_Z));
    TERN_(Z2_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Z2, STR_Z2));
    TERN_(Z3_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Z3, STR_Z3));
    TERN_(Z4_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(Z4, STR_Z4));
    TERN_( I_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(I,  STR_I));
    TERN_( J_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(J,  STR_J));
    TERN_( K_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(K,  STR_K));
    TERN_( U_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(U,  STR_U));
    TERN_( V_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(V,  STR_V));
    TERN_( W_HAS_HOME_CURRENT, TMC_EDIT_HOMING_CURRENT(W,  STR_W));
    END_MENU();
  }

#endif // EDITABLE_HOMING_CURRENT

#if HAS_STEALTHCHOP

  #define TMC_EDIT_STEP_MODE(ST, STR) EDIT_ITEM_F(bool, F(STR), &stepper##ST.stored.stealthChop_enabled, []{ stepper##ST.refresh_stepping_mode(); })

  void menu_tmc_step_mode() {
    START_MENU();
    STATIC_ITEM(MSG_TMC_STEALTHCHOP);
    BACK_ITEM(MSG_TMC_DRIVERS);
    TERN_( X_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(X,  STR_X));
    TERN_(X2_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(X2, STR_X2));
    TERN_( Y_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Y,  STR_Y));
    TERN_(Y2_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Y2, STR_Y2));
    TERN_( Z_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Z,  STR_Z));
    TERN_(Z2_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Z2, STR_Z2));
    TERN_(Z3_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Z3, STR_Z3));
    TERN_(Z4_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(Z4, STR_Z4));
    TERN_(E0_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E0, STR_E0));
    TERN_(E1_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E1, STR_E1));
    TERN_(E2_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E2, STR_E2));
    TERN_(E3_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E3, STR_E3));
    TERN_(E4_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E4, STR_E4));
    TERN_(E5_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E5, STR_E5));
    TERN_(E6_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E6, STR_E6));
    TERN_(E7_HAS_STEALTHCHOP, TMC_EDIT_STEP_MODE(E7, STR_E7));
    END_MENU();
  }

#endif // HAS_STEALTHCHOP

void menu_tmc() {
  START_MENU();
  BACK_ITEM(MSG_ADVANCED_SETTINGS);
  SUBMENU(MSG_TMC_CURRENT, menu_tmc_current);
  TERN_(HYBRID_THRESHOLD,        SUBMENU(MSG_TMC_HYBRID_THRS, menu_tmc_hybrid_thrs));
  TERN_(SENSORLESS_HOMING,       SUBMENU(MSG_TMC_HOMING_THRS, menu_tmc_homing_thrs));
  TERN_(EDITABLE_HOMING_CURRENT, SUBMENU(MSG_TMC_HOMING_CURRENT, menu_tmc_homing_current));
  TERN_(HAS_STEALTHCHOP,         SUBMENU(MSG_TMC_STEALTHCHOP, menu_tmc_step_mode));
  END_MENU();
}

#endif // HAS_TRINAMIC_CONFIG
