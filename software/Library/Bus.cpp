#include "Bus.h"

// #include <Encoding/COBS.h>
#include <PacketSerial.h>
#include <FastCRC.h>

#include "Util.h"

FastCRC8 CRC8;

// void receive() {
//   Packetizer::parse();
// }

Bus::Bus(Stream& upper_serial, Stream& lower_serial):
  upper_serial_(upper_serial), lower_serial_(lower_serial)
  // task_receive_(RECEIVE_PERIOD_MS * TASK_MILLISECOND,
  //               TASK_FOREVER, &receive, &scheduler, true)
{
  message_count_ = 0;
  error_count_ = 0;
  overflowed_ = false;
  upper_received_ = 0;
  lower_received_ = 0;
}

bool Bus::begin() {

  // upper_serial_.begin(BUS_SERIAL_BAUD);
  // lower_serial_.begin(BUS_SERIAL_BAUD);

  // task_receive_.set(RECEIVE_PERIOD_MS * TASK_MILLISECOND,
  //                   TASK_FOREVER, &receive);

  // Packetizer::subscribe
  //   (upper_serial_,
  //    [&](const uint8_t* data, const size_t size) {
  //      receiveDataFromUpper(data, size);
  //    });
  // Packetizer::subscribe
  //   (lower_serial_,
  //    [&](const uint8_t* data, const size_t size) {
  //      receiveDataFromLower(data, size);
  //    });

  return true;
}


void Bus::send(const Message& message) {
  channel_.tx.push(message);
  uint8_t buf[BUFFER_SIZE];
  unsigned len;

  while ((len = channel_.write(buf)) > 0) {
    // Packetizer::send(lower_serial_, data, size);
    // Packetizer::send(upper_serial_, data, size);

    buf[len] = CRC8.smbus(buf, len);

    uint8_t encoded[BUFFER_SIZE + 2];
    unsigned encoded_len = COBS::encode(buf, len + 1, encoded);
    encoded[encoded_len] = 0;

    upper_serial_.write(encoded, encoded_len + 1);
    lower_serial_.write(encoded, encoded_len + 1);


    Serial.print("TX     ");
    printlnBytes(buf, len, 5);
    Serial.print("TX enc ");
    printlnBytes(encoded, encoded_len + 1);
  }
}



void Bus::receive() {

  while (upper_serial_.available() > 0) {
    uint8_t data = upper_serial_.read();

    if (upper_received_ + 1 < BUFFER_SIZE) {
      upper_buf_[upper_received_] = data;
      upper_received_++;
    }
    else {
      overflowed_ = true;
      error_count_++;
      upper_received_ = 0;
      Serial.println("Overflowed");
    }

    if (data == 0) {

      Serial.print("RX     ");
      printlnBytes(upper_buf_, upper_received_ - 1);

      uint8_t decoded[BUFFER_SIZE + 2];
      unsigned len = COBS::decode(upper_buf_, upper_received_ - 1, decoded);

      if (upper_received_ == 0 || len == 0) {
        error_count_++;
      }
      else {
        uint8_t crc8 = CRC8.smbus(decoded, len - 1);

        if (crc8 != decoded[len - 1]) {
          error_count_++;
          Serial.println("CRC Error");
        }
        else {
          lower_serial_.write(upper_buf_, upper_received_);

          Serial.print("RX dec ");
          printlnBytes(decoded, len - 1, 5);

          overflowed_ = false;

          receiveData(decoded, len - 1);
        }
      }

      upper_received_ = 0;
    }
  }

  while (lower_serial_.available() > 0) {
    uint8_t data = lower_serial_.read();

    Serial.print(data, HEX);
    Serial.print(" ");

    if (lower_received_ + 1 < BUFFER_SIZE) {
      lower_buf_[lower_received_] = data;
      lower_received_++;
    }
    else {
      overflowed_ = true;
      error_count_++;
      lower_received_ = 0;
      Serial.println("Overflowed");
    }

    if (data == 0) {

      Serial.print("RX     ");
      printlnBytes(lower_buf_, lower_received_ - 1);


      uint8_t decoded[BUFFER_SIZE + 2];
      unsigned len = COBS::decode(lower_buf_, lower_received_ - 1, decoded);

      if (lower_received_ == 0 || len == 0) {
        error_count_++;
      }
      else {
        uint8_t crc8 = CRC8.smbus(decoded, len - 1);

        if (crc8 != decoded[len - 1]) {
          error_count_++;
          Serial.println("CRC Error");
        }
        else {
          upper_serial_.write(lower_buf_, lower_received_);

          Serial.print("RX dec ");
          printlnBytes(decoded, len - 1, 5);

          overflowed_ = false;

          receiveData(decoded, len - 1);
        }
      }

      lower_received_ = 0;
    }
  }

}


// void Bus::receiveDataFromUpper(const uint8_t* data, const size_t size) {
//   Packetizer::send(lower_serial_, data, size);

//   receiveData(data, size);
// }
// void Bus::receiveDataFromLower(const uint8_t* data, const size_t size) {
//   Packetizer::send(lower_serial_, data, size);

//   receiveData(data, size);
// }

void Bus::receiveData(const uint8_t* data, const size_t size) {
  channel_.read(data, size);

  for (const auto& f : listeners_) {
    f(channel_.rx);
  }

  auto range = specified_listeners_.equal_range(channel_.rx.id);
  for (auto itr = range.first; itr != range.second; itr++) {
    itr->second(channel_.rx);
  }

  message_count_++;
}
