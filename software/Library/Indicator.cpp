#include "Indicator.h"

#include "Arduino.h"

Indicator* instance;

void blinkOff() {
  digitalWrite(instance->status_pin_, LOW);
}

void blinkOffError() {
  digitalWrite(instance->error_pin_, instance->error_ != ErrorStatus::NoError);
}

Indicator::Indicator(uint8_t status_pin, uint8_t error_pin):
  status_pin_(status_pin), error_pin_(error_pin),
  task_blink_(0, TASK_ONCE, &blinkOff, &scheduler, false),
  task_blink_error_(0, TASK_ONCE, &blinkOffError, &scheduler, false) {

  instance = this;
}

bool Indicator::begin() {
  pinMode(status_pin_, OUTPUT);
  pinMode(error_pin_, OUTPUT);

  error_ = ErrorStatus::Error;

  digitalWrite(status_pin_, LOW);
  digitalWrite(error_pin_, HIGH);

  return true;
}


void Indicator::setError(ErrorStatus error) {
  error_ = error;
  digitalWrite(error_pin_, error != ErrorStatus::NoError);
}

void Indicator::clearError() {
  error_ = ErrorStatus::NoError;
  digitalWrite(error_pin_, LOW);
}

void Indicator::blink(unsigned sustain_ms) {
  digitalWrite(status_pin_, HIGH);

  task_blink_.setInterval(sustain_ms);
  task_blink_.restartDelayed();
}

void Indicator::errorEvent(unsigned sustain_ms) {
  digitalWrite(error_pin_, HIGH);
  task_blink_error_.setInterval(sustain_ms);
  task_blink_error_.restartDelayed();
}
