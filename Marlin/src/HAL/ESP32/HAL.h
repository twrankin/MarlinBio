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

/**
 * HAL for Espressif ESP32 WiFi
 */

#define CPU_32_BIT

#include <stdint.h>

#include "../shared/Marduino.h"
#include "../shared/math_32bit.h"
#include "../shared/HAL_SPI.h"

#include "fastio.h"
#include "i2s.h"

#if ENABLED(WIFISUPPORT)
  #include "wifi/WebSocketSerial.h"
#endif

#if ENABLED(ESP3D_WIFISUPPORT)
  #include <esp3dlib.h>
#endif

#include "FlushableHardwareSerial.h"

// ------------------------
// Defines
// ------------------------

#define MYSERIAL1 flushableSerial

#if ENABLED(ESP3D_WIFISUPPORT)
  typedef ForwardSerial1Class< decltype(Serial2Socket) > DefaultSerial1;
  extern DefaultSerial1 MSerial0;
  #define MYSERIAL2 MSerial0
#elif ENABLED(WIFISUPPORT)
  #define MYSERIAL2 webSocketSerial
#endif

#define CRITICAL_SECTION_START() portENTER_CRITICAL(&hal.spinlock)
#define CRITICAL_SECTION_END()   portEXIT_CRITICAL(&hal.spinlock)

#define HAL_CAN_SET_PWM_FREQ   // This HAL supports PWM Frequency adjustment
#define PWM_FREQUENCY  1000u   // Default PWM frequency when set_pwm_duty() is called without set_pwm_frequency()
#define PWM_RESOLUTION   10u   // Default PWM bit resolution
#define CHANNEL_MAX_NUM  15u   // max PWM channel # to allocate (7 to only use low speed, 15 to use low & high)
#define MAX_PWM_IOPIN    33u   // hardware pwm pins < 34
#ifndef MAX_EXPANDER_BITS
  #define MAX_EXPANDER_BITS 32 // I2S expander bit width (max 32)
#endif

// ------------------------
// Types
// ------------------------

typedef double isr_float_t;   // FPU ops are used for single-precision, so use double for ISRs.
typedef int16_t pin_t;

typedef struct pwm_pin {
  uint32_t pwm_cycle_ticks = 1000000UL / (PWM_FREQUENCY) / 4; // # ticks per pwm cycle
  uint32_t pwm_tick_count = 0;  // current tick count
  uint32_t pwm_duty_ticks = 0;  // # of ticks for current duty cycle
} pwm_pin_t;

class Servo;
typedef Servo hal_servo_t;

// ------------------------
// Public functions
// ------------------------

//
// Tone
//
void tone(const pin_t _pin, const unsigned int frequency, const unsigned long duration=0);
void noTone(const pin_t _pin);
int8_t get_pwm_channel(const pin_t pin, const uint32_t freq, const uint16_t res);
void analogWrite(const pin_t pin, const uint16_t value, const uint32_t freq=PWM_FREQUENCY, const uint16_t res=8);

//
// Pin Mapping for M42, M43, M226
//
#define GET_PIN_MAP_PIN(index) index
#define GET_PIN_MAP_INDEX(pin) pin
#define PARSED_PIN_INDEX(code, dval) parser.intval(code, dval)

#if ENABLED(USE_ESP32_EXIO)
  void Write_EXIO(uint8_t IO, uint8_t v);
#endif

//
// Delay in cycles (used by DELAY_NS / DELAY_US)
//
FORCE_INLINE static void DELAY_CYCLES(uint32_t x) {
  unsigned long start, ccount, stop;

  /**
   * It's important to care for race conditions (and overflows) here.
   * Race condition example: If `stop` calculates to being close to the upper boundary of
   * `uint32_t` and if at the same time a longer loop interruption kicks in (e.g. due to other
   * FreeRTOS tasks or interrupts), `ccount` might overflow (and therefore be below `stop` again)
   * without the loop ever being able to notice that `ccount` had already been above `stop` once
   * (and that therefore the number of cycles to delay has already passed).
   * As DELAY_CYCLES (through DELAY_NS / DELAY_US) is used by software SPI bit banging to drive
   * LCDs and therefore might be called very, very often, this seemingly improbable situation did
   * actually happen in reality. It resulted in apparently random print pauses of ~17.9 seconds
   * (0x100000000 / 240 MHz) or multiples thereof, essentially ruining the current print by causing
   * large blobs of filament.
   */

  __asm__ __volatile__ ( "rsr     %0, ccount" : "=a" (start) );
  stop = start + x;
  ccount = start;

  if (stop >= start) {
    // no overflow, so only loop while in between start and stop:
    // 0x00000000 -----------------start****stop-- 0xFFFFFFFF
    while (ccount >= start && ccount < stop) {
      __asm__ __volatile__ ( "rsr     %0, ccount" : "=a" (ccount) );
    }
  }
  else {
    // stop did overflow, so only loop while outside of stop and start:
    // 0x00000000 **stop-------------------start** 0xFFFFFFFF
    while (ccount >= start || ccount < stop) {
      __asm__ __volatile__ ( "rsr     %0, ccount" : "=a" (ccount) );
    }
  }

}

// ------------------------
// Class Utilities
// ------------------------

#pragma GCC diagnostic push
#if GCC_VERSION <= 50000
  #pragma GCC diagnostic ignored "-Wunused-function"
#endif

int freeMemory();

#pragma GCC diagnostic pop

// ------------------------
// MarlinHAL Class
// ------------------------

#define HAL_ADC_VREF_MV   3300
#define HAL_ADC_RESOLUTION  10

class MarlinHAL {
public:

  // Earliest possible init, before setup()
  MarlinHAL() {}

  // Watchdog
  static void watchdog_init()    IF_DISABLED(USE_WATCHDOG, {});
  static void watchdog_refresh() IF_DISABLED(USE_WATCHDOG, {});

  static void init() {}        // Called early in setup()
  static void init_board();    // Called less early in setup()
  static void reboot();        // Restart the firmware

  // Interrupts
  static portMUX_TYPE spinlock;
  static bool isr_state() { return spinlock.owner == portMUX_FREE_VAL; }
  static void isr_on()  { if (spinlock.owner != portMUX_FREE_VAL) portEXIT_CRITICAL(&spinlock); }
  static void isr_off() { portENTER_CRITICAL(&spinlock); }

  static void delay_ms(const int ms) { delay(ms); }

  // Tasks, called from idle()
  static void idletask();

  // Reset
  static uint8_t get_reset_source();
  static void clear_reset_source() {}

  // Free SRAM
  static int freeMemory();

  static pwm_pin_t pwm_pin_data[MAX_EXPANDER_BITS];

  //
  // ADC Methods
  //

  static uint16_t adc_result;

  // Called by Temperature::init once at startup
  static void adc_init();

  // Called by Temperature::init for each sensor at startup
  static void adc_enable(const pin_t pin) {}

  // Begin ADC sampling on the given pin. Called from Temperature::isr!
  static void adc_start(const pin_t pin);

  // Is the ADC ready for reading?
  static bool adc_ready() { return true; }

  // The current value of the ADC register
  static uint16_t adc_value() { return adc_result; }

  /**
   * If not already allocated, allocate a hardware PWM channel
   * to the pin and set the duty cycle..
   * Optionally invert the duty cycle [default = false]
   * Optionally change the scale of the provided value to enable finer PWM duty control [default = 255]
   */
  static void set_pwm_duty(const pin_t pin, const uint16_t v, const uint16_t v_size=255, const bool invert=false);

  /**
   * Allocate and set the frequency of a hardware PWM pin
   * Returns -1 if no pin available.
   */
  static int8_t set_pwm_frequency(const pin_t pin, const uint32_t f_desired);

};
