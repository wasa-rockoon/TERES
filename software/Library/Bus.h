#pragma once

#include <map>
#include <Arduino.h>
#include <TaskScheduler.h>
#include <Packetizer.h>
#include "Message.h"


#define BUS_SERIAL_BAUD 115200

#define RECEIVE_PERIOD_MS 1
#define SEND_PERIOD_MS 1

using MessageCallback = std::function<void(const Message&)>;

extern Scheduler scheduler;

class Bus_ {

public:

  bool begin(Stream& upper_serial, Stream& lower_serial);

  void listen(const MessageCallback& func);
  void listen(uint8_t id, const MessageCallback& func);

  void send(const Message& message);

  inline unsigned getErrorCount();
  inline unsigned getMessageCount();

  static Bus_& getInstance();

private:
  Bus_();

  BinaryChannel channel_;

  Stream& upper_serial_ = Serial;
  Stream& lower_serial_ = Serial;

  std::multimap<uint8_t, const MessageCallback&> specified_listeners_;
  std::vector<const MessageCallback&> listeners_;

  unsigned message_count_;

  Task task_receive_;

  void receiveDataFromUpper(const uint8_t* data, const size_t size);
  void receiveDataFromLower(const uint8_t* data, const size_t size);
  void receiveData(const uint8_t* data, const size_t size);

  friend void receiveDataFromUpper_(const uint8_t* data, const size_t size);
  friend void receiveDataFromLower_(const uint8_t* data, const size_t size);
};

extern Bus_& Bus;
