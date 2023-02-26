#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include <Message.h>
#include <Bus.h>

// Pins
#define TWE_RESET_PIN 19
#define TWE_PGM_PIN 9
#define TWE_TX_PIN 15
#define TWE_RX_PIN 14
#define TWE_SLEEP_PIN 18
#define TWE_RTS_PIN 10

#define TWE_BUF_SIZE 256


extern Scheduler scheduler;

class TWELITE {
public:
  TWELITE(Stream& serial);

  bool begin(bool reset = true);

  void send(const Message& message);
  void send();

  void listen(MessageCallback func);

  inline void setDestination(uint8_t id) { dest_id_ = id; };

  inline int getLQI() const { return lqi_; }
  inline int getReceivedCount() const { return received_count_; }
  inline int getSentCount() const { return sent_count_; }
  inline int getErrorCount() const { return error_count_ + dropped_count_; }

private:
  Stream& serial_;
  BinaryChannel channel_;

  Task task_update_;

  MessageCallback callback_;

  uint8_t dest_id_;

  uint8_t rx_buf_[TWE_BUF_SIZE];
  unsigned rx_count_;

  int lqi_;

  unsigned sent_count_;
  unsigned received_count_;
  unsigned dropped_count_;
  unsigned error_count_;

  void receive();

  inline void error() { error_count_++; }

  friend void update();
};
