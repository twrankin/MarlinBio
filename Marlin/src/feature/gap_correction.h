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

#pragma once

#include "../inc/MarlinConfig.h"

/// MarlinBio: The number of channels to monitor on the capacitance sensor.
/// Redefined here just for visibility.
#define GC_CHANNEL_NUM _GC_CHANNEL_NUM

/// MarlinBio: If this many sensor readings are exactly the same, something
/// is probably wrong with the sensor.
#define GC_VARIANCE_CHECK_LIMIT 50

/// MarlinBio: The number of attempts to make when correcting gaps before giving up.
#define GC_ATTEMPT_LIMIT 255

class FDC2214;

/**
 * MarlinBio:
 * @brief Gap correction class
 * @details Contains data and logic for gap correction.
 */
class GapCorrection {
  friend class GcodeSuite;
  friend class MarlinSettings;

  static bool sampling;

  static float capacitance_initial[GC_CHANNEL_NUM];
  static float capacitance_current[GC_CHANNEL_NUM];

  static uint8_t variance_checks[GC_CHANNEL_NUM];
  static float   capacitance_variance[GC_CHANNEL_NUM];

  static float touch_threshold_factor;

  static feedRate_t feedrate;

  static FDC2214 sensor;

public:

  #if GC_DEBUG
    static uint16_t stream_secs;
    static millis_t last_report;
  #endif

private:

  static void start_sampling();
  static void end_sampling();

  static float capacitance_to_mm(float capacitance);

public:

  static void init();

  static float x_plus_initial()      {return capacitance_initial[GC_X_PLUS_CHANNEL];}
  static float x_plus_capacitance()  {return capacitance_current[GC_X_PLUS_CHANNEL];}
  static float x_minus_initial()     {return capacitance_initial[GC_X_MINUS_CHANNEL];}
  static float x_minus_capacitance() {return capacitance_current[GC_X_MINUS_CHANNEL];}
  static float y_plus_initial()      {return capacitance_initial[GC_Y_PLUS_CHANNEL];}
  static float y_plus_capacitance()  {return capacitance_current[GC_Y_PLUS_CHANNEL];}
  static float y_minus_initial()     {return capacitance_initial[GC_Y_MINUS_CHANNEL];}
  static float y_minus_capacitance() {return capacitance_current[GC_Y_MINUS_CHANNEL];}

  static void calibrate();
  static void update_capacitances(bool verify=false);
  static void branch_point();

  static void print_capacitances();

  #if GC_DEBUG
    static void debug_stream(millis_t time=millis());
  #endif
};

extern GapCorrection gapCorrection;
