#include "Bus.h"


void receive() {
  Packetizer::parse();
}

void receiveDataFromUpper_(const uint8_t* data, const size_t size) {
  Bus.getInstance().receiveDataFromUpper(data, size);
}
void receiveDataFromLower_(const uint8_t* data, const size_t size) {
  Bus.getInstance().receiveDataFromUpper(data, size);
}

Bus_::Bus_(): task_receive_(RECEIVE_PERIOD_MS * TASK_MILLISECOND,
                            TASK_FOREVER, &receive, &scheduler, true) {
  message_count_ = 0;
}

bool Bus_::begin(Stream& upper_serial, Stream& lower_serial) {
  upper_serial_ = upper_serial;
  lower_serial_ = lower_serial;

  upper_serial_.begin(BUS_SERIAL_BAUD);
  lower_serial_.begin(BUS_SERIAL_BAUD);

  task_receive_.set(RECEIVE_PERIOD_MS * TASK_MILLISECOND,
                    TASK_FOREVER, &receive);

  Packetizer::subscribe(upper_serial_, receiveDataFromUpper_);
  Packetizer::subscribe(lower_serial_, receiveDataFromLower_);

  return true;
}



void Bus_::send(const Message& message) {
  channel_.tx.push(message);
  uint8_t data[256];
  unsigned size;
  while ((size = channel_.write(data)) > 0) {
    Packetizer::send(lower_serial_, data, size);
    Packetizer::send(upper_serial_, data, size);
  }
}


unsigned Bus_::getErrorCount() {
  return upper_.getErrorCount() + lower_.getErrorCount();
};


void Bus_::receiveDataFromUpper(const uint8_t* data, const size_t size) {
  Packetizer::send(lower_serial_, data, size);

  receiveData(data, size);
}
void Bus_::receiveDataFromLower(const uint8_t* data, const size_t size) {
  Packetizer::send(lower_serial_, data, size);

  receiveData(data, size);
}

void Bus_::receiveData(const uint8_t* data, const size_t size) {
  channel_.read(data, size);

  for (const auto& f : listeners_) {
    f(channel_.rx);
  }

  auto range = specified_listeners_.equal_range(channel_.rx.id);
  for (auto itr = range.first; itr != range.second; itr++) {
    *itr(channel_.rx);
  }

  message_count++;
}

Bus_& Bus_::getInstance() {
  static Bus_ instance = Bus_();
  return instance;
}

Bus_& Bus = Bus.getInstance();
