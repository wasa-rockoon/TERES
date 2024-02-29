/*
 * uart.cpp
 *
 *  Created on: Aug 19, 2022
 *      Author: yuuki.fj
 */

#include "uart.hpp"

#include <cstring>

UART::UART(HardwareSerial& serial, uint8_t* buf,
    unsigned ring_size, unsigned tail_size):
	serial_(serial), buf_(buf), ring_size_(ring_size), tail_size_(tail_size),
	rx_read_ptr_(0), rx_write_ptr_(0) {};

bool UART::begin() {
  return true;
}

unsigned UART::available() {
  while (serial_.available()) {
    buf_[rx_write_ptr_] = serial_.read();
    // Serial.print(buf_[rx_write_ptr_]);
    rx_write_ptr_++;
  	if (rx_write_ptr_ == ring_size_) rx_write_ptr_ = 0;
  }
	return (rx_write_ptr_ + ring_size_ - rx_read_ptr_) % ring_size_;
};

int16_t UART::read() {
  while (serial_.available()) {
    buf_[rx_write_ptr_] = serial_.read();
    // Serial.print(buf_[rx_write_ptr_]);
    rx_write_ptr_++;
  	if (rx_write_ptr_ == ring_size_) rx_write_ptr_ = 0;
  }

	if (rxIsEmpty()) return -1;

	uint8_t c = buf_[rx_read_ptr_];

	rx_read_ptr_++;
	if (rx_read_ptr_ == ring_size_) rx_read_ptr_ = 0;

	return c;
}

int16_t UART::peek(unsigned pos) {
  while (serial_.available()) {
    buf_[rx_write_ptr_] = serial_.read();
    // Serial.print(buf_[rx_write_ptr_]);
    rx_write_ptr_++;
  	if (rx_write_ptr_ == ring_size_) rx_write_ptr_ = 0;
  }

  if (rxIsEmpty()) return -1;

  return buf_[(rx_read_ptr_ + pos) % ring_size_];
}


uint8_t* UART::read(unsigned len) {
  while (serial_.available()) {
    buf_[rx_write_ptr_] = serial_.read();
    // Serial.print(buf_[rx_write_ptr_]);
    rx_write_ptr_++;
  	if (rx_write_ptr_ == ring_size_) rx_write_ptr_ = 0;
  }

  if (len > available()) return nullptr;

  unsigned ptr = rx_read_ptr_;

  if (ptr + len >= ring_size_) {
    std::memcpy(buf_ + ring_size_, buf_, ptr + len - ring_size_);
    rx_read_ptr_ = ptr + len - ring_size_;
  }
  else {
    rx_read_ptr_ += len;
  }

  return buf_ + ptr;
}


void UART::clear() {
	rx_read_ptr_ = rx_write_ptr_;
}

void UART::changeBaudRate(unsigned baud) {
  // serial_.flush();
  // delay(100);
  // serial_.end();
  // delay(100);
  // serial_.begin(baud);
}

