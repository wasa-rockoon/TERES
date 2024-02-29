#pragma once

#include <map>
#include <vector>
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
  void unlisten();

  void send(const Message& message);
  void send();

  void receive();

  inline unsigned getErrorCount() const {
    return upper_channel_.getErrorCount() + lower_channel_.getErrorCount()
      + error_count_ + dropped_count_;
  }
  inline unsigned getMessageCount() const { return message_count_; }

private:

  BinaryChannel upper_channel_;
  BinaryChannel lower_channel_;

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
  unsigned dropped_count_;

  bool overflowed_;

  Task task_resend_;

  void send(BinaryChannel& channel, Stream& serial, const Message& message);
  void send(BinaryChannel& channel, Stream& serial);

  void receive(BinaryChannel& channel, BinaryChannel& another_channel,
               Stream& serial, Stream& another_serial, uint8_t *buf,
               unsigned& received);

  // void receiveData(const uint8_t* data, const size_t size);
};

