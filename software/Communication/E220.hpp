/*
 * E220.hpp
 *
 *  Created on: Jul 10, 2023
 *      Author: yuuki.fj
 */

#ifndef INC_E220_HPP_
#define INC_E220_HPP_

#include "Arduino.h"

#include "uart.hpp"

#define LORA_TX_PIN 12
#define LORA_RX_PIN 13
#define LORA_AUX_PIN 11
#define LORA_M0_PIN 7
#define LORA_M1_PIN 8

#define E220_BROADCAST 0xFFFF

#ifndef E220_TIMEOUT_MS
#define E220_TIMEOUT_MS 100
#endif

#define E220_PARAM_MAX 8
#define E220_PACKET_LEN_MAX 128

class E220: public UART {
public:

  enum class Mode : uint8_t {
    NORMAL = 0b00,
    WOR_TX = 0b01,
    WOR_RX = 0b10,
    CONFIG_DS = 0b11,
  };

  enum class ADDR : uint8_t {
    ADDH = 0x00,
    ADDL = 0x01,
    REG0 = 0x02,
    REG1 = 0x03,
    REG2 = 0x04,
    REG3 = 0x05,
    CRYPT_H = 0x06,
    CRYPT_L = 0x07,
    VERSION = 0x08,
  };

  enum class SF : uint8_t {
    SF5 = 0b000,
    SF6 = 0b001,
    SF7 = 0b010,
    SF8 = 0b011,
    SF9 = 0b100,
    SF10 = 0b101,
    SF11 = 0b110,
  };

  enum class BW : uint8_t {
    BW125kHz = 0b00,
    BW250kHz = 0b01,
    BW500kHz = 0x10,
  };

  enum class Power : uint8_t {
    POWER13dBm = 0b01,
    POWER7dBm = 0b10,
    POWER0dBm = 0b11,
  };

  enum class SendMode : uint8_t {
    TRANSPARENT = 0b0,
    FIXED = 0b1,
  };

  E220(HardwareSerial& serial, uint8_t* buf,
      unsigned ring_size, unsigned tail_size
      );

  bool begin();
  bool setMode(Mode mode);

  bool isBusy();

  bool sendTransparent(const uint8_t* data, unsigned len);
  bool send(uint16_t addr, uint8_t channel, const uint8_t* data, unsigned len);

  unsigned receive(uint8_t*& data);

  bool setModuleAddr(uint16_t addr);
  bool setSerialBaudRate(unsigned baud);
  bool setDataRate(SF sf, BW bw);
  bool setEnvRSSIEnable(bool enable);
  bool setPower(Power power);
  bool setChannel(uint8_t channel);
  bool setRSSIEnable(bool enable);
  bool setSendMode(SendMode mode);

  bool setParametersToDefault();

  inline uint8_t getRSSI() const { return rssi_; };
  uint8_t getEnvRSSI();

  bool writeRegister(ADDR addr, const uint8_t* parameters, uint8_t len = 1);
  bool writeRegisterWithMask(ADDR addr, uint8_t mask, uint8_t value);
  bool readRegister(ADDR addr, uint8_t* parameters, uint8_t len = 1);

private:
  bool RSSI_enabled_;
  unsigned baud_;
  uint8_t rssi_;
};


#endif /* INC_E220_HPP_ */
