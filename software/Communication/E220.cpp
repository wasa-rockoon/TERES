/*
 * E220.cpp
 *
 *  Created on: Jul 10, 2023
 *      Author: yuuki.fj
 */

#include "E220.hpp"


E220::E220(HardwareSerial& serial, uint8_t* buf,
      unsigned ring_size, unsigned tail_size
      ): UART(serial, buf, ring_size, tail_size) {
  baud_ = 9600;
  RSSI_enabled_ = false;
  rssi_ = 255;
}

bool E220::begin() {
  bool ok = UART::begin();
  serial_.begin(9600);

  pinMode(LORA_M0_PIN, OUTPUT);
  pinMode(LORA_M1_PIN, OUTPUT);
  pinMode(LORA_AUX_PIN, INPUT);

  setMode(Mode::NORMAL);
  while (isBusy());
  delay(200);
  return ok;
}

bool E220::isBusy() {
  return !digitalRead(LORA_AUX_PIN);
}

bool E220::sendTransparent(const uint8_t* data, unsigned len) {
  if (isBusy()) return false;

  uint8_t buf[E220_PACKET_LEN_MAX + 1];
  buf[0] = len;
  memcpy(buf + 1, data, len);
  return write(buf, 1 + len);
}

bool E220::send(uint16_t addr, uint8_t channel, const uint8_t* data, unsigned len) {
  if (isBusy()) return false;

  uint8_t buf[E220_PACKET_LEN_MAX + 4];
  buf[0] = (uint8_t)(addr >> 8);
  buf[1] = (uint8_t)addr;
  buf[2] = channel;
  buf[3] = len;
  memcpy(buf + 4, data, len);
  return write(buf, 4 + len);
}

unsigned E220::receive(uint8_t*& data) {
  if (available() == 0) return 0;
  uint8_t len = peek();
  if (RSSI_enabled_) {
    if ((int)available() < len + 2) return 0;
    len = read();
    data = read(len);
    rssi_ = 256 - (uint8_t)read();
  }
  else {
    if ((int)available() < len + 1) return 0;
    len = read();
    data = read(len);
  }
  return len;
}


bool E220::setMode(Mode mode) {
  if (mode == Mode::CONFIG_DS) changeBaudRate(9600);
  else changeBaudRate(baud_);

  digitalWrite(LORA_M0_PIN, static_cast<uint8_t>(mode) & 0b01 ? HIGH : LOW);
  digitalWrite(LORA_M1_PIN, static_cast<uint8_t>(mode) & 0b10 ? HIGH : LOW);
  delay(100);
  while (isBusy());

  clear();

  return true;
}

bool E220::setSerialBaudRate(unsigned baud) {
  uint8_t bits = 0b000;
  switch (baud) {
  case 1200:
    bits = 0b000;
    break;
  case 2400:
    bits = 0b001;
    break;
  case 4800:
    bits = 0b010;
    break;
  case 9600:
    bits = 0b011;
    break;
  case 19200:
    bits = 0b100;
    break;
  case 38400:
    bits = 0b101;
    break;
  case 57600:
    bits = 0b100;
    break;
  case 115200:
    bits = 0b111;
    break;
  default:
    return false;
  }

  if (writeRegisterWithMask(ADDR::REG0, 0b11100000, bits << 5)) baud_ = baud;
  
  return true;
}

bool E220::setModuleAddr(uint16_t addr) {
  uint8_t addr_high_low[2] = { (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF) };
  return writeRegister(ADDR::ADDH, addr_high_low, 2);
}

bool E220::setDataRate(SF sf, BW bw) {
  return writeRegisterWithMask(
      ADDR::REG0, 0b00011111,
      static_cast<uint8_t>(sf) << 2 | static_cast<uint8_t>(bw)
  );
}

bool E220::setEnvRSSIEnable(bool enable) {
  return writeRegisterWithMask(ADDR::REG1, 0b00100000, (enable ? 0b1 : 0b0) << 5);
}

bool E220::setPower(Power power) {
  bool ok = true;
  uint8_t reg1;
  ok &= readRegister(ADDR::REG1, &reg1);
  reg1 = (reg1 & 0b00000011) | static_cast<uint8_t>(power);
  ok &= writeRegister(ADDR::REG1, &reg1);
  return ok;
}

bool E220::setChannel(uint8_t channel) {
  return writeRegister(ADDR::REG2, &channel);
}

bool E220::setRSSIEnable(bool enable) {
  bool ok = writeRegisterWithMask(ADDR::REG3, 0b10000000, (enable ? 0b1 : 0b0) << 7);
  if (ok) RSSI_enabled_ = enable;
  return ok;
}

bool E220::setSendMode(SendMode mode) {
  return writeRegisterWithMask(ADDR::REG3, 0b01000000, static_cast<uint8_t>(mode) << 6);
}

uint8_t E220::getEnvRSSI() {
  bool ok = true;
  uint8_t cmd[6] = { 0xC0, 0xC1, 0xC2, 0xC3, 0x00, 0x02 };
  ok &= write(cmd, 6);

  unsigned long ms = millis();
  while (isBusy() || (int)available() < 5) {
    if (millis() - ms > E220_TIMEOUT_MS) return 255;
  }

  uint8_t* rx = read(5);
  clear();

  cmd[3] = 0xC1;
  ok &= memcmp(cmd + 3, rx, 3) == 0;

  // rssi_ = 256 - rx[4];
  return 256 - rx[3];
}

bool E220::setParametersToDefault() {
  uint8_t parameters[] = { 0x00, 0x00, 0x62, 0x00, 0x0F, 0x03, 0x00, 0x00 };
  return writeRegister(ADDR::ADDH, parameters, 8);
}

bool E220::writeRegister(ADDR addr, const uint8_t* parameters, uint8_t len) {
  bool ok = true;

  clear();

  uint8_t cmd[16];
  cmd[0] = 0xC0;
  cmd[1] = static_cast<uint8_t>(addr);
  cmd[2] = len;
  memcpy(cmd + 3, parameters, len);
  ok &= write(cmd, 3 + len);

  unsigned long ms = millis();
  while (isBusy() || (int)available() < 3 + len) {
    if (millis() - ms > E220_TIMEOUT_MS) return false;
  }

  uint8_t* rx = read(3 + len);
  clear();

  cmd[0] = 0xC1;
  ok &= memcmp(cmd, rx, 3 + len) == 0;
//  ok &= memcmp(parameters, rx + 3, len) == 0;

  delay(10);

  return ok;
}

bool E220::writeRegisterWithMask(ADDR addr, uint8_t mask, uint8_t value) {
  bool ok = true;
  uint8_t reg;
  ok &= readRegister(addr, &reg);
  reg = (reg & ~mask) | (value & mask);
  ok &= writeRegister(addr, &reg);
  return ok;
}

bool E220::readRegister(ADDR addr, uint8_t* parameters, uint8_t len) {
  bool ok = true;

  clear();

  uint8_t header[3] = { 0xC1, static_cast<uint8_t>(addr), len };
  ok &= write(header, 3);

  unsigned long ms = millis();

  while ((int)available() < 3 + len) {
    if (millis() - ms > E220_TIMEOUT_MS) return false;
  }

  uint8_t* rx = read(3 + len);
  clear();

  ok &= memcmp(header, rx, 3) == 0;
  if (ok) memcpy(parameters, rx + 3, len);

  while (isBusy()) {
    if (millis() - ms > E220_TIMEOUT_MS) return false;
  }

  delay(10);

  return ok;
}
