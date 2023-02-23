#pragma once

#include <map>
#include <functional>
#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include "Message.h"


#define BUS_SERIAL_BAUD 115200

#define BUFFER_SIZE 256


using MessageCallback = std::function<void(const Message&)>;

extern Scheduler scheduler;

class Bus {

public:

  Bus(Stream& upper_serial, Stream& lower_serial);

  bool begin();

  void listen(MessageCallback func);
  void listen(uint8_t id, MessageCallback func);

  void send(const Message& message);

  void receive();

  inline unsigned getErrorCount() {
    return channel_.getErrorCount() + error_count_;
  }
  inline unsigned getMessageCount() { return message_count_; }

private:

  BinaryChannel channel_;

  Stream& upper_serial_;
  Stream& lower_serial_;

  uint8_t upper_buf_[BUFFER_SIZE];
  uint8_t lower_buf_[BUFFER_SIZE];

  unsigned upper_received_;
  unsigned lower_received_;

  std::multimap<uint8_t, MessageCallback> specified_listeners_;
  std::vector<MessageCallback> listeners_;

  unsigned message_count_;
  unsigned error_count_;

  bool overflowed_;

  Task task_receive_;


  // void receiveDataFromUpper(const uint8_t* data, const size_t size);
  // void receiveDataFromLower(const uint8_t* data, const size_t size);
  void receiveData(const uint8_t* data, const size_t size);

  // friend void receiveDataFromUpper_(const uint8_t* data, const size_t size);
  // friend void receiveDataFromLower_(const uint8_t* data, const size_t size);
};

