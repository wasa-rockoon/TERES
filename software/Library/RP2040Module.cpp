#ifdef ARDUINO_ARCH_RP2040

#include "RP2040Module.h"

#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include <EEPROM.h>

#include <Bus.hpp>


RP2040Module::RP2040Module(uint8_t node_name)
  : bus(node_name, Serial2, Serial1),
    indicator(LED_STATUS_PIN, LED_ERROR_PIN) {}

bool RP2040Module::begin() {
  Serial.begin(SERIAL_BAUD);

  EEPROM.begin(1);
      // #ifdef DEBUG
      //   while (!Serial);
      // #endif

  Serial1.setTX(TX0_PIN);
  Serial1.setRX(RX0_PIN);
  Serial1.setFIFOSize(BUS_FIFO_SIZE);
  Serial1.begin(BUS_SERIAL_BAUD);

  Serial2.setTX(TX1_PIN);
  Serial2.setRX(RX1_PIN);
  Serial2.setFIFOSize(BUS_FIFO_SIZE);
  Serial2.begin(BUS_SERIAL_BAUD);

  bool bus_ok = bus.begin();
  bool indicator_ok = indicator.begin();

  bus_error_count_ = bus.getErrorCount();

  rp2040.wdt_begin(WDT_DURATION);

  return bus_ok && indicator_ok;
}

void RP2040Module::update() {
  bus.update();

  if (bus.getErrorCount() != bus_error_count_) {
    indicator.errorEvent();
    bus_error_count_ = bus.getErrorCount();
  }

  rp2040.wdt_reset();
}

extern unsigned getMillis() {
  return millis();
}

uint8_t readEEPROM(unsigned addr) { return EEPROM.read(addr); }
void writeEEPROM(unsigned addr, uint8_t value) {
  EEPROM.write(addr, value);
  EEPROM.commit();
}
unsigned getUnique() { return rp2040.hwrand32(); }

#endif
