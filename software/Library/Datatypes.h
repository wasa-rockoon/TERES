#pragma once

#include "Arduino.h"

enum class FlightMode : uint8_t {
  STANDBY,
  FLIGHT,
  RECOVERY,
};

enum class FlightSequence : uint8_t {
  BEFORE_FLIGHT,
  BURNING,
  ASCENDING,
  DESCENDING,
  LANDED,
};

struct Rocket {
  FlightMode mode : 2;
  FlightSequence sequence : 3;
};
