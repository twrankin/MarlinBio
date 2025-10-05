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

#include "../MarlinCore.h"

#if NEED_FDC2214

#include "FDC2214.h"
#include <Wire.h>
#include "../libs/hex_print.h"

/// MarlinBio: Write a register over I2C.
bool FDC2214::write16(uint8_t reg, uint16_t value) {
  bool ret;

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  ret = Wire.endTransmission() == 0;

  if (!ret) {
    SERIAL_ECHOLN("FDC2214 write failed.");
  }
  return ret;
}

/// MarlinBio: Read a register over I2C.
bool FDC2214::read16(uint8_t reg, uint16_t &value) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  /// MarlinBio: Repeated start.
  if (Wire.endTransmission(false) != 0) {
    SERIAL_ECHOLN("FDC2214 read failed.");
    return false;
  }

  /// MarlinBio: Read two bytes.
  uint8_t toRead = 2;
  uint32_t start = millis();

  Wire.requestFrom((int)I2C_ADDR, (int)toRead);
  while (Wire.available() < toRead) {
    if (millis() - start > 20) {
      SERIAL_ECHOLN("FDC2214 read failed, no reply.");
      return false;
    }
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
  uint16_t reg = regDrive(ch);
  uint16_t val = (uint16_t(idrive) << DriveCurrentBits::CH_IDRIVE_SHIFT);
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

bool FDC2214::reset() {
  return write16(REG_RESET_DEV, ResetDevBits::RESET_DEV);
}

bool FDC2214::checkSleep(bool &asleep) {
  uint16_t cfg;
  if (!read16(REG_CONFIG, cfg)) return false;
  asleep = bool(cfg & ConfigBits::SLEEP_MODE_EN);
  return true;
}

/// MarlinBio: Read raw 28-bit DATA for channel (0..3). Returns false on I2C error.
bool FDC2214::readData28(uint8_t ch, uint32_t &data28) {
  if (ch > 3) return false;

  /// MarlinBio: Poll the ready flag until a conversion is available.
  uint16_t status;
  int      retries = 10;
  while(retries--) {
    if (!read16(REG_STATUS, status)) return false;
    if (status & unreadConv(ch)) break;
    safe_delay(10);
  }
  if (!(status & unreadConv(ch))) return false;

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

  reset();

  /// MarlinBio: 0x5449 = "TI" in ASCII.
  #define TI_MANUFACTURER_ID 0x5449
  #define FDC221x_DEVICE_ID  0x3055
  /// MarlinBio: Read the manufacturer and device IDs to confirm communication.
  uint16_t manufacturerID, deviceID;
  int retries = 10;
  while (retries--) {
    if (!readIDs(manufacturerID, deviceID)) return false;
    if (manufacturerID == TI_MANUFACTURER_ID && deviceID == FDC221x_DEVICE_ID) break;
    safe_delay(500);
  }
  if (manufacturerID != TI_MANUFACTURER_ID || deviceID != FDC221x_DEVICE_ID) {
    return false;
  }

  /// MarlinBio: Ensure the device is in sleep mode to allow configuration changes.
  /// It is supposed to be in sleep after reset.
  bool asleep = false;
  if (!checkSleep(asleep) || !asleep) return false;

  /// MarlinBio: Set the reference clock to use an external source.
  if (!setRefClockExternal()) return false;

  /// MarlinBio: Set the number of channels.
  if (!setMultiChannel(_GC_CHANNEL_NUM)) return false;

  /// MarlinBio: Set the deglitch low pass filter cutoff.
  if (!setDeglitch(deglitch)) return false;

  for (int ch = 0; ch < _GC_CHANNEL_NUM; ch++) {
    /// MarlinBio: Set up the clocks.
    if (!setClockDividers(ch, fref_div, fin_sel)) return false;

    /// MarlinBio: Set up timing.
    if (!setRcount(ch, rcount)) return false;
    if (!setSettleCount(ch, settlecount)) return false;

    /// MarlinBio: Set the circuit drive current.
    if (!setIDrive(ch, idrive)) return false;
  }

  /// MarlinBio: Disable interrupts for now.
  if (!setIntbEnabled(false)) return false;

  /// MarlinBio: Note: It won't start sampling data until wake is called.

  return true;
}

bool FDC2214::sleep() {
  uint16_t cfg;
  if (!read16(REG_CONFIG, cfg)) return false;
  cfg |= ConfigBits::SLEEP_MODE_EN;
  return write16(REG_CONFIG, cfg);
}

bool FDC2214::wake() {
  uint16_t cfg;
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
  if (denom <= 0) {
    return false;
  }

  /// MarlinBio: Convert to pF.
  const double C = 1e12 / denom;
  capacitance = static_cast<float>(C);

  /// MarlinBio: Uncomment this to print the values.
  //SERIAL_ECHOLN("Channel: ", channel, " DATAx: ", DATAx, " fSENSORx: ", fSENSORx, " capacitance: ", capacitance);

  return true;
}

#if GC_DEBUG
  void FDC2214::dump() {
    SERIAL_ECHOLN("Dumping FDC2214 registers.");

    uint16_t data;
    for (int reg = 0; reg <= REG_DEVICE_ID; reg++) {
      /// MarlinBio: These addresses are skipped for some reason.
      if (reg == 0x1d || (reg > REG_DRIVE_CURRENT_CH3 && reg < REG_MANUFACTURER_ID)) continue;

      if (read16(reg, data)) {
        SERIAL_ECHO("Register: 0x");
        print_hex_word(reg);
        SERIAL_ECHO(" Data: 0x");
        print_hex_word(data);
        SERIAL_EOL();
      }
    }
  }
#endif

#endif // NEED_FDC2214
