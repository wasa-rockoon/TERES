#ifdef ARDUINO_ARCH_RP2040

#include "RP2040Module.h"
#include "Bus.h"

#include <Arduino.h>


RP2040Module *rp2040_instance;

void wdt_reset(){
  rp2040.wdt_reset();
}

RP2040Module::RP2040Module()
    : bus(Serial2, Serial1), indicator(LED_STATUS_PIN, LED_ERROR_PIN),
      task_wdt_(WDT_DURATION / 4 * TASK_MILLISECOND, TASK_FOREVER, wdt_reset,
                &scheduler, false) {
  rp2040_instance = this;
}

bool RP2040Module::begin() {
  Serial.begin(SERIAL_BAUD);
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

  task_wdt_.enable();
  rp2040.wdt_begin(WDT_DURATION);

  return true;
}

void serialEvent1() {
  rp2040_instance->bus.receive();

  if (rp2040_instance->bus.getErrorCount() != rp2040_instance->bus_error_count_) {
    rp2040_instance->indicator.errorEvent();
    rp2040_instance->bus_error_count_ = rp2040_instance->bus.getErrorCount();
  }
}

void serialEvent2() {
  rp2040_instance->bus.receive();

  if (rp2040_instance->bus.getErrorCount() != rp2040_instance->bus_error_count_) {
    rp2040_instance->indicator.errorEvent();
    rp2040_instance->bus_error_count_ = rp2040_instance->bus.getErrorCount();
  }
}


#endif
