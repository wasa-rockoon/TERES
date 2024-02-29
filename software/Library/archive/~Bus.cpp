#include "Bus.h"

// #define DEBUG

// #include <Encoding/COBS.h>
#include <PacketSerial.h>
#include <FastCRC.h>

#include "Message.h"
#include "Util.h"

FastCRC8 CRC8;

Bus* bus_instance;


void resend() {
  bus_instance->send();
  // Serial.print("R");
}

Bus::Bus(Stream& upper_serial, Stream& lower_serial):
  upper_serial_(upper_serial), lower_serial_(lower_serial),
  task_resend_(TASK_IMMEDIATE,
                TASK_FOREVER, resend, &scheduler, false)
{
  message_count_ = 0;
  error_count_ = 0;
  dropped_count_ = 0;
  overflowed_ = false;
  upper_received_ = 0;
  lower_received_ = 0;

  bus_instance = this;
}

bool Bus::begin() {

  return true;
}

void Bus::listen(MessageCallback func) {
  listeners_.push_back(func);
}

void Bus::listen(uint8_t id, MessageCallback func) {
  specified_listeners_.insert(std::make_pair(id, func));
}

void Bus::unlisten() {
  listeners_.clear();
  specified_listeners_.clear();
}

void Bus::send() {
  send(upper_channel_, upper_serial_);
  send(lower_channel_, lower_serial_);
}

void Bus::send(const Message& message) {
  send(upper_channel_, upper_serial_, message);
  send(lower_channel_, lower_serial_, message);
}

void Bus::send(BinaryChannel &channel, Stream &serial, const Message &message) {
  if (channel.tx.isFull()) {
    channel.tx.pop();
    dropped_count_++;
#ifdef DEBUG
    Serial.print("DROPPED");
#endif
  }
  channel.tx.push(message);

#ifdef DEBUG
  message.print();
#endif

  send(channel, serial);
}

void Bus::send(BinaryChannel& channel, Stream& serial) {
  unsigned len;

  while ((len = channel.nextWriteSize()) > 0) {
    if (!serial.availableForWrite()) {
      // Serial.printf("a: %d\n", serial.availableForWrite());
      task_resend_.enable();
      return;
    }

    uint8_t buf[BUFFER_SIZE];

    len = channel.write(buf);

    buf[len] = CRC8.smbus(buf, len);

    uint8_t encoded[BUFFER_SIZE + 2];
    unsigned encoded_len = COBS::encode(buf, len + 1, encoded);
    encoded[encoded_len] = 0;

    serial.write(encoded, encoded_len + 1);

    // Serial.print("TX     ");
    // printlnBytes(buf, len);
    // Serial.print("TX enc ");
    // printlnBytes(encoded, encoded_len + 1);
  }

  if (upper_channel_.tx.isEmpty() && lower_channel_.tx.isEmpty()) {
    task_resend_.disable();
  }
}

void Bus::receive() {
  // Serial.printf("R\n");
  receive(upper_channel_, lower_channel_, upper_serial_, lower_serial_,
          upper_buf_, upper_received_);
  receive(lower_channel_, upper_channel_, lower_serial_, upper_serial_,
          lower_buf_, lower_received_);
}

void Bus::receive(BinaryChannel& channel, BinaryChannel& another_channel,
                  Stream& serial, Stream &another_serial, uint8_t *buf,
                  unsigned& received) {
  while (serial.available() > 0) {
    uint8_t data = serial.read();

    // Serial.printf("%02X", data);

    buf[received] = data;
    received++;

    if (received + 1 >= BUFFER_SIZE) {
      overflowed_ = true;
      error_count_++;
      received = 0;
#ifdef DEBUG
      Serial.println("Overflowed");
#endif
      return;
    }

    if (data == 0) {
#ifdef DEBUG
      // Serial.print("RX     ");
      // printlnBytes(buf, received - 1);
#endif

      uint8_t decoded[BUFFER_SIZE + 2];
      unsigned len = COBS::decode(buf, received - 1, decoded);

      if (received == 0 || len == 0) {
        error_count_++;

#ifdef DEBUG
        Serial.println("No Data");
#endif
        continue;
      }

      uint8_t crc8 = CRC8.smbus(decoded, len - 1);

      if (crc8 != decoded[len - 1]) {
        error_count_++;
#ifdef DEBUG
        Serial.println("CRC Error");
#endif
        continue;
      }

      // Serial.print("\nRX dec ");
      // printlnBytes(decoded, len - 1);

      channel.read(decoded, len - 1);

#ifdef DEBUG
      channel.rx.print();
#endif

      send(another_channel, another_serial, channel.rx);

      for (const auto &f : listeners_) {
        f(channel.rx);
      }

      auto range = specified_listeners_.equal_range(channel.rx.id);
      for (auto itr = range.first; itr != range.second; itr++) {
        itr->second(channel.rx);
      }

      message_count_++;
      received = 0;
      overflowed_ = false;
    }
  }
}

