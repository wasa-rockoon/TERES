#pragma once

#include <TaskSchedulerDeclarations.h>

extern Scheduler scheduler;

enum class ErrorStatus {
  NoError,
    Warning,
    Error,
    };

class Indicator {

 public:
  Indicator(uint8_t status_pin, uint8_t error_pin);

  bool begin();

  void blink(unsigned sustain_ms = 100);

  void errorEvent(unsigned sustain_ms = 200);
  void setError(ErrorStatus error);
  void clearError();

 private:
  uint8_t status_pin_;
  uint8_t error_pin_;

  ErrorStatus error_;

  Task task_blink_;
  Task task_blink_error_;

  friend void blinkOff();
  friend void blinkOffError();
};
