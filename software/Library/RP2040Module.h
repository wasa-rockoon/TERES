#pragma once

#include "Bus.h"
#include "Indicator.h"

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
#define BUS_FIFO_SIZE 128

class RP2040Module {

public:
  RP2040Module();

  bool begin();

  Bus bus;
  Indicator indicator;

private:
  unsigned bus_error_count_;

  friend void serialEvent1();
  friend void serialEvent2();
};
