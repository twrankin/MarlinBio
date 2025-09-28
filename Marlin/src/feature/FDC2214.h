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

#if NEED_FDC2214

#include <Wire.h>

/// MarlinBio: Disclaimer: I had to assemble this pretty quickly, feel free to question
/// any design decisions and change anything that looks suspect.
/// Make sure to have a good understanding of the FDC2214 datasheet before
/// reaching out with any questions: https://www.ti.com/lit/ds/symlink/fdc2214.pdf.
/// The following TODOs are not planned, they are for anyone to do when they are needed.
/// TODO: Periodically monitor STATUS for errors.
/// TODO: Enable all errors in ERROR_CONFIG and check the error bits in the DATA_CHx registers before reading.
/// TODO: Set INTB_DIS to 0, connect the the INTB pin to a GPIO and create an ISR for it, and enable the INTB errors in ERROR_CONFIG.
/// TODO: After the INTB ISR is setup, switch from polling to interrupts for data.
/// TODO: Allow a variable number of channels and variable settings per channel.
/// TODO: Add logging, especially for errors.

/// MarlinBio: Settings for the FDC2214 that are used below during configuration.
/// They are just #defines here, to make it easier for people to quickly iterate
/// as we find the ideal settings.

/// The I2C address for the device.
/// 0x2A when the ADDR pin is low, 0x2B when it is high. This will need
/// to be split out when using multiple boards if they share SCL/SDA lines.
#define I2C_ADDR 0x2A

/// MarlinBio: These are defined in the board's pins file, to be more portable.
//#define FDC_SDA_PIN PC10
//#define FDC_SCL_PIN PC11

/// MarlinBio: The maximum clock allowed is 400kbps.
#define I2C_CLK 400000

/// MarlinBio: The inductance of the LC circuit.
#define INDUCTANCE 0.000018

/// MarlinBio: The frequency of the external clock source.
#define FCLK 40000000

/// MarlinBio: The value for CHx_FIN_SEL; should be 2 for single ended (most common),
/// 1 for differential with resonance frequencies between 0.01MHz and 8.75MHz,
/// and 2 for differential with resonance frequencies between 5MHz and 10MHz.
#define FIN_SEL 2

/// MarlinBio: How much to divide the clock to get the reference frequency.
#define FREF_DIV 1

/// MarlinBio: The sensor drive current. Should be set such that 1.2V ≤ sensor voltage amplitude ≤ 1.8V.
/// See the DRIVE_CURRENT_CHx register descriptions for values.
#define IDRIVE 0b01111

/// MarlinBio: The amount of time to allow the oscillation to settle after activation.
/// The time is calculated by: time = SETTLECOUNT * 16 / fref.
#define SETTLECOUNT 0x0400

/// MarlinBio: The amount of time to allow for sampling, longer = higher resolution.
/// The time is calculated by: time = RCOUNT * 16 / fref.
#define RCOUNT 0xffff

/// MarlinBio: The frequency cutoff for the low pass filter.
/// Frequencies above this are ignored, to filter out noise.
/// So this just needs to be set above the highest possible resonant frequency.
#define DEGLITCH FDC2214::Deglitch::DEGLITCH_10MHZ

class FDC2214 {

  /// MarlinBio: The register map.
  enum : uint8_t {
    /// MarlinBio: Data registers (28-bit: must read MSB first then LSB).
    REG_DATA_CH0_MSB = 0x00,
    REG_DATA_CH0_LSB = 0x01,
    REG_DATA_CH1_MSB = 0x02,
    REG_DATA_CH1_LSB = 0x03,
    REG_DATA_CH2_MSB = 0x04,
    REG_DATA_CH2_LSB = 0x05,
    REG_DATA_CH3_MSB = 0x06,
    REG_DATA_CH3_LSB = 0x07,

    /// MarlinBio: Conversion timing.
    REG_RCOUNT_CH0 = 0x08,
    REG_RCOUNT_CH1 = 0x09,
    REG_RCOUNT_CH2 = 0x0A,
    REG_RCOUNT_CH3 = 0x0B,

    /// MarlinBio: Frequency offset (not used).
    REG_OFFSET_CH0 = 0x0C,
    REG_OFFSET_CH1 = 0x0D,
    REG_OFFSET_CH2 = 0x0E,
    REG_OFFSET_CH3 = 0x0F,

    /// MarlinBio: Sensor activation settle time.
    REG_SETTLECOUNT_CH0 = 0x10,
    REG_SETTLECOUNT_CH1 = 0x11,
    REG_SETTLECOUNT_CH2 = 0x12,
    REG_SETTLECOUNT_CH3 = 0x13,

    /// MarlinBio: Dividers: FIN_SEL and FREF_DIV.
    REG_CLOCK_DIVIDERS_CH0 = 0x14,
    REG_CLOCK_DIVIDERS_CH1 = 0x15,
    REG_CLOCK_DIVIDERS_CH2 = 0x16,
    REG_CLOCK_DIVIDERS_CH3 = 0x17,

    /// MarlinBio: Status.
    REG_STATUS       = 0x18,
    REG_ERROR_CONFIG = 0x19,

    /// MarlinBio: Config and mux.
    REG_CONFIG     = 0x1A,
    REG_MUX_CONFIG = 0x1B,

    /// MarlinBio: Reset.
    REG_RESET_DEV = 0x1C,

    /// MarlinBio: Drive current.
    REG_DRIVE_CURRENT_CH0 = 0x1E,
    REG_DRIVE_CURRENT_CH1 = 0x1F,
    REG_DRIVE_CURRENT_CH2 = 0x20,
    REG_DRIVE_CURRENT_CH3 = 0x21,

    /// MarlinBio: Expect 0x5449 ("TI") for the manufacturer, and 0x3055 for the device ID.
    REG_MANUFACTURER_ID = 0x7E,
    REG_DEVICE_ID       = 0x7F
  };

  /// MarlinBio: CONFIG bits (partial)
  struct ConfigBits {
    /// MarlinBio: 15:14 ACTIVE_CHAN: active channel in single-channel mode.
    static constexpr uint16_t ACTIVE_CHAN_SHIFT = 14;
    /// MarlinBio: 13 SLEEP_MODE_EN: 1=sleep, 0=active.
    static constexpr uint16_t SLEEP_MODE_EN = (1u<<13);
    /// MarlinBio: 11 SENSOR_ACTIVATE_SEL: 0=full current (recommended).
    static constexpr uint16_t SENSOR_ACTIVATE_SEL = (1u<<11);
    /// MarlinBio: 9 REF_CLK_SRC: 0=internal, 1=external.
    static constexpr uint16_t REF_CLK_SRC = (1u<<9);
    /// MarlinBio: 0 INTB_DIS: 1=disable, 0=enable.
    static constexpr uint16_t INTB_DIS = (1u<<7);
    /// MarlinBio: 6 HIGH_CURRENT_DRV: High current drive mode (channel 0 only).
    static constexpr uint16_t HIGH_CURRENT_DRV = (1u<<6);
  };

  /// MarlinBio: MUX_CONFIG bits.
  struct MuxBits {
    /// MarlinBio: 15 AUTOSCAN_EN: 0=single channel, 1=multi-channel.
    static constexpr uint16_t AUTOSCAN_EN = (1u<<15);
    /// MarlinBio: 14:13 RR_SEQUENCE: Which channels to sample, b00=0,1 ; b01=0..2 ; b10=0..3.
    static constexpr uint16_t RR_SEQUENCE_SHIFT = 13;
    /// MarlinBio: 10:8 DEGLITCH: b001=1MHz, b100=3.3MHz, b101=10MHz, b011=33MHz.
    static constexpr uint16_t DEGLITCH_SHIFT = 0;
  };

  /// MarlinBio: CLOCK_DIVIDERS_CHx bits.
  struct ClockDivBits {
    /// MarlinBio: 13:12 FIN_SEL: sensor frequency divider.
    /// b01: ÷1 (for ~0.01–8.75 MHz differential).
    /// b10: ÷2 (for 5–10 MHz differential or 0.01–10 MHz single-ended).
    static constexpr uint16_t FIN_SEL_SHIFT = 12;
    /// MarlinBio: 9:0 FREF_DIV: reference frequency divider.
    /// fREFx = fCLK / FREF_DIV.
    static constexpr uint16_t FREF_DIV_MASK = 0x03FF;
  };

  /// MarlinBio: STATUS bits (partial).
  struct StatusBits {
    static constexpr uint16_t DRDY_MASK = 0x0008;
  };

  /// MarlinBio: Deglitch enumerations for convenience.
  enum Deglitch : uint8_t {
    DEGLITCH_1MHZ  = 0b001,
    DEGLITCH_3_3MHZ= 0b100,
    DEGLITCH_10MHZ = 0b101,
    DEGLITCH_33MHZ = 0b011
  };

  /// MarlinBio: Register helpers.
  static inline uint8_t regDataMSB(uint8_t ch) { return (uint8_t)(REG_DATA_CH0_MSB + (ch<<1)); }
  static inline uint8_t regDataLSB(uint8_t ch) { return (uint8_t)(REG_DATA_CH0_LSB + (ch<<1)); }
  static inline uint8_t regRcount (uint8_t ch) { return (uint8_t)(REG_RCOUNT_CH0 + ch); }
  static inline uint8_t regSettle (uint8_t ch) { return (uint8_t)(REG_SETTLECOUNT_CH0 + ch); }
  static inline uint8_t regClockDiv(uint8_t ch){ return (uint8_t)(REG_CLOCK_DIVIDERS_CH0 + ch); }
  static inline uint8_t regDrive  (uint8_t ch) { return (uint8_t)(REG_DRIVE_CURRENT_CH0 + ch); }

  /// MarlinBio: Write a register over I2C.
  bool write16(uint8_t reg, uint16_t value);

  /// MarlinBio: Read a register over I2C.
  bool read16(uint8_t reg, uint16_t &value);

  /// MarlinBio: Read raw 28-bit DATA for channel (0..3).
  bool readData28(uint8_t ch, uint32_t &data28);

  /// MarlinBio: Set the reference clock to external.
  bool setRefClockExternal();

  /// MarlinBio: Set multi-channel round-robin.
  bool setMultiChannel(uint8_t num);

  /// MarlinBio: Set the deglitch filter value.
  bool setDeglitch(Deglitch d);

  /// MarlinBio: Clock dividers for a channel: FREF divider (1..1023) and FIN selector (÷1 or ÷2)
  bool setClockDividers(uint8_t ch, uint16_t fref_div, uint8_t fin_sel);

  /// MarlinBio: Amount of time to allow for monitoring the oscillations and calculating
  /// the frequency. A longer time results in more resolution, but lower samples per second.
  bool setRcount(uint8_t ch, uint16_t rcount);

  /// MarlinBio: Amount of time the oscillation is allowed to settle before conversions start
  /// after activation.
  bool setSettleCount(uint8_t ch, uint16_t settle);

  /// MarlinBio: This should be set such that the target sensor voltage amplitude is ~1.2–1.8 Vpk.
  bool setIDrive(uint8_t ch, uint8_t idrive);

  /// MarlinBio: Enable/disable INTB.
  bool setIntbEnabled(bool enabled);

  /// MarlinBio: Read the status register.
  bool readStatus(uint16_t &status);

public:

  /// MarlinBio: Initialize the FDC2214.
  /// This should only need to be done when the IC is powered on or after a reset.
  bool init(uint8_t fin_sel = FIN_SEL, uint16_t fref_div = FREF_DIV, uint16_t rcount = RCOUNT, uint16_t settlecount = SETTLECOUNT, uint8_t idrive = IDRIVE, Deglitch deglitch = DEGLITCH);

  /// MarlinBio: Put the device into sleep.
  /// The device starts in sleep mode, and must be in sleep mode for
  /// any configuration changes.
  bool sleep();

  /// MarlinBio: Exit sleep.
  /// This will begin sampling according to the configuration.
  /// Return to sleep mode to change settings, or when sampling isn't currently needed.
  bool wake();

  /// MarlinBio: Read IDs to verify presence.
  bool readIDs(uint16_t &manufacturer, uint16_t &device) {
    return read16(REG_MANUFACTURER_ID, manufacturer) && read16(REG_DEVICE_ID, device);
  }

  /// MarlinBio: Read a value from the sensor for a channel and convert it to capacitance in pF.
  bool read_channel(uint8_t channel, float &capacitance);
};

#endif /// MarlinBio: NEED_FDC2214
