#include "Arduino.h"
#include <UARTBus.hpp>

#include <EEPROM.h>

unsigned getMillis() {
  return millis();
}

void enableInterrupts() {
  
}

void disableInterrupts() {
  
}


uint8_t readEEPROM(unsigned addr) { return EEPROM.read(addr); }
void writeEEPROM(unsigned addr, uint8_t value) {
  EEPROM.write(addr, value);
  EEPROM.commit();
}
unsigned getUnique() { return ESP.getEfuseMac(); }
