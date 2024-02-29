#pragma once

#include "Arduino.h"
#include "Indicator.h"
#include "UARTBus.hpp"

// Pin assign
#define LED_STATUS_PIN 0
#define LED_ERROR_PIN 25
#define SWITCH_PIN 23
#define TX0_PIN 16
#define RX0_PIN 17
#define TX1_PIN 4
#define RX1_PIN 5

// Constants
#define SERIAL_BAUD 115200
#define BUS_FIFO_SIZE 4096

#define WDT_DURATION 5000

#define ROCKET 0x1

class RP2040Module {

public:
  RP2040Module(uint8_t node_name);

  bool begin();

  UARTBus bus;
  Indicator indicator;

  void update();

private:
  unsigned bus_error_count_;
};
