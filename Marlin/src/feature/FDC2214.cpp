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

#include "../inc/MarlinConfig.h"

#if NEED_FDC2214

#include "FDC2214.h"
#include <Wire.h>

/// MarlinBio: Write a register over I2C.
bool FDC2214::write16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  return (Wire.endTransmission() == 0);
}

/// MarlinBio: Read a register over I2C.
bool FDC2214::read16(uint8_t reg, uint16_t &value) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  /// MarlinBio: Repeated start.
  if (Wire.endTransmission(false) != 0) return false;

  /// MarlinBio: Read two bytes
  uint8_t toRead = 2;
  uint32_t start = millis();
  Wire.requestFrom((int)I2C_ADDR, (int)toRead);
  while (Wire.available() < toRead) {
    if (millis() - start > 20) return false;
    delayMicroseconds(100);
  }
  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  value = (uint16_t(hi) << 8) | lo;
  return true;
}

bool FDC2214::setSettleCount(uint8_t ch, uint16_t settle) {
  if (ch > 3) return false;
  return write16(regSettle(ch), settle);
}

bool FDC2214::setRcount(uint8_t ch, uint16_t rcount) {
  if (ch > 3 || rcount < 0x0100) return false;
  return write16(regRcount(ch), rcount);
}

bool FDC2214::setIDrive(uint8_t ch, uint8_t idrive) {
  if (ch > 3 || idrive > 0x1F) return false;
  SERIAL_ECHOLN("Setting iDrive: ", idrive);
  uint16_t reg = regDrive(ch);
  uint16_t val = (uint16_t(idrive) << 11);
  return write16(reg, val);
}

bool FDC2214::setDeglitch(Deglitch d) {
  uint16_t mux;
  if (!read16(REG_MUX_CONFIG, mux)) return false;
  mux &= ~(0b111u << MuxBits::DEGLITCH_SHIFT);
  mux |= (uint16_t(d) << MuxBits::DEGLITCH_SHIFT);
  return write16(REG_MUX_CONFIG, mux);
}

bool FDC2214::setClockDividers(uint8_t ch, uint16_t fref_div, uint8_t fin_sel) {
  if (ch > 3 || fref_div == 0 || fref_div > 1023 || fin_sel == 0 || fin_sel > 2) return false;
  uint16_t val = (uint16_t(fin_sel) << ClockDivBits::FIN_SEL_SHIFT) | (fref_div & ClockDivBits::FREF_DIV_MASK);
  return write16(regClockDiv(ch), val);
}

bool FDC2214::setRefClockExternal() {
  uint16_t cfg;
  if (!read16(REG_CONFIG, cfg)) return false;
  cfg |= ConfigBits::REF_CLK_SRC;
  return write16(REG_CONFIG, cfg);
}

bool FDC2214::setMultiChannel(uint8_t num) {
  if (num < 2 || num > 4) return false;
  uint16_t mux;
  if (!read16(REG_MUX_CONFIG, mux)) return false;
  mux |= MuxBits::AUTOSCAN_EN;
  mux &= ~(0b11u << MuxBits::RR_SEQUENCE_SHIFT);
  uint8_t code = (num == 2 ? 0b00 : num == 3 ? 0b01 : 0b10);
  mux |= (uint16_t(code) << MuxBits::RR_SEQUENCE_SHIFT);
  return write16(REG_MUX_CONFIG, mux);
}

bool FDC2214::setIntbEnabled(bool enabled) {
  uint16_t cfg;
  if (!read16(REG_CONFIG, cfg)) return false;
  if (enabled) cfg &= ~ConfigBits::INTB_DIS;
  else         cfg |=  ConfigBits::INTB_DIS;
  return write16(REG_CONFIG, cfg);
}

bool FDC2214::readStatus(uint16_t &status) {
  return read16(REG_STATUS, status);
}

/// MarlinBio: Read raw 28-bit DATA for channel (0..3). Returns false on I2C error.
bool FDC2214::readData28(uint8_t ch, uint32_t &data28) {
  if (ch > 3) return false;
  uint8_t msbReg = regDataMSB(ch);
  uint8_t lsbReg = regDataLSB(ch);

  uint16_t msb;
  if (!read16(msbReg, msb)) return false;

  uint16_t lsb;
  if (!read16(lsbReg, lsb)) return false;

  /// MarlinBio: Upper 12 bits in MSB register [27:16], lower 16 in LSB [15:0]
  uint32_t upper12 = (msb & 0x0FFF);
  data28 = (upper12 << 16) | lsb;
  return true;
}

bool FDC2214::init(uint8_t fin_sel, uint16_t fref_div, uint16_t rcount, uint16_t settlecount, uint8_t idrive, Deglitch deglitch) {
  Wire.begin(uint32_t(FDC_SDA_PIN), uint32_t(FDC_SCL_PIN));
  /// MarlinBio: Up to 400kHz according to the datasheet.
  Wire.setClock(I2C_CLK);

  /// MarlinBio: Put the device into sleep mode (if it wasn't already) to allow configuration changes.
  if (!sleep()) return false;

  /// MarlinBio: Set the reference clock to use an external source.
  if (!setRefClockExternal()) return false;

  /// MarlinBio: Set up the clocks.
  if (!setClockDividers(0, fref_div, fin_sel)) return false;
  if (!setClockDividers(1, fref_div, fin_sel)) return false;

  /// MarlinBio: Two channels.
  if (!setMultiChannel(2)) return false;

  /// MarlinBio: Set the deglitch low pass filter cutoff.
  if (!setDeglitch(deglitch)) return false;

  /// MarlinBio: Timing
  if (!setRcount(0, rcount)) return false;
  if (!setRcount(1, rcount)) return false;
  if (!setSettleCount(0, settlecount)) return false;
  if (!setSettleCount(1, settlecount)) return false;

  /// MarlinBio: Set the circuit drive current.
  if (!setIDrive(0, idrive)) return false;
  if (!setIDrive(1, idrive)) return false;

  /// MarlinBio: Disable interrupts for now.
  if (!setIntbEnabled(false)) return false;

  /// MarlinBio: 0x5449 = "TI" in ASCII.
  #define TI_MANUFACTURER_ID 0x5449
  #define FDC221x_DEVICE_ID  0x3055

  uint16_t manufacturerID, deviceID;
  if (!readIDs(manufacturerID, deviceID)) return false;
  if (manufacturerID != TI_MANUFACTURER_ID || deviceID != FDC221x_DEVICE_ID) {
    SERIAL_ECHOLN("Bad sensor values: ", manufacturerID, "/", deviceID);
    return false;
  }

  return true;
}

bool FDC2214::sleep() {
  uint16_t cfg;
  SERIAL_ECHOLN("Sleeping");
  if (!read16(REG_CONFIG, cfg)) return false;
  cfg |= ConfigBits::SLEEP_MODE_EN;
  return write16(REG_CONFIG, cfg);
}

bool FDC2214::wake() {
  uint16_t cfg;
  SERIAL_ECHOLN("Waking");
  if (!read16(REG_CONFIG, cfg)) return false;
  cfg &= ~ConfigBits::SLEEP_MODE_EN;
  return write16(REG_CONFIG, cfg);
}

/// MarlinBio: Convert a value from the DATA registers to a frequency.
/// According to the datasheet: fSENSORx = CHx_FIN_SEL * fREFx * DATAx / 2^28.
/// Additionally, fREFx = fCLK / CHx_FREF_DIVIDER.
bool FDC2214::read_channel(uint8_t channel, float &capacitance) {
  /// MarlinBio: Precalculate (2π)^2 and CHx_FIN_SEL * fREFx / 2^28.
  constexpr double TWO_PI_SQUARED = 39.47841760435743;
  constexpr double K = (static_cast<double>(FIN_SEL) * (static_cast<double>(FCLK) / FREF_DIV)) / 268435456.0;

  uint32_t DATAx = 0;
  if (!readData28(channel, DATAx)) {
    return false;
  }

  /// MarlinBio: K * DATAx == CHx_FIN_SEL * fREFx * DATAxx / 2^28.
  const double fSENSORx = K * DATAx;

  /// MarlinBio: According to the datasheet (and also the formula for LC tank circuits):
  /// C = 1 / L * (2 * π * fSENSORx) ^ 2.
  const double denom = static_cast<double>(INDUCTANCE) * TWO_PI_SQUARED * fSENSORx * fSENSORx;
  if (!(denom > 0.0)) {
    return false;
  }

  const double C = 1.0 / denom;
  capacitance = static_cast<float>(C);
  return true;
}

#endif // NEED_FDC2214
