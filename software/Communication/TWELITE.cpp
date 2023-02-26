#include "TWELITE.h"

TWELITE *twelite_instance;

void no_callback(const Message &message) {}

void update() {
  twelite_instance->receive();
  twelite_instance->send();
}

TWELITE::TWELITE(Stream &serial)
    : serial_(serial), callback_(no_callback),
      task_update_(TASK_IMMEDIATE, TASK_FOREVER, update, &scheduler, false) {
  twelite_instance = this;
  lqi_ = -1;
}

bool TWELITE::begin(bool reset) {
  pinMode(TWE_RESET_PIN, OUTPUT);
  pinMode(TWE_PGM_PIN, OUTPUT);
  pinMode(TWE_SLEEP_PIN, OUTPUT);
  pinMode(TWE_RTS_PIN, INPUT);

  digitalWrite(TWE_PGM_PIN, HIGH);
  digitalWrite(TWE_RESET_PIN, HIGH);
  digitalWrite(TWE_SLEEP_PIN, HIGH);

  if (reset) {
    delay(100);
    digitalWrite(TWE_RESET_PIN, LOW);
    delay(100);
    digitalWrite(TWE_RESET_PIN, HIGH);
    delay(100);
    if (serial_.available() == 0) return false;
  }

  while (serial_.available() > 0) serial_.read();

  task_update_.enable();

  return true;
}

void TWELITE::send(const Message &message) {
  if (channel_.tx.isFull()) {
    channel_.tx.pop();
    dropped_count_++;
  }
  channel_.tx.push(message);
  send();
}

void TWELITE::send() {
  unsigned len;

  while ((len = channel_.nextWriteSize()) > 0) {
    if (digitalRead(TWE_RTS_PIN)) {
      Serial.println("STOP");
      return;
    }

    uint8_t buf[TWE_BUF_SIZE];

    buf[0] = dest_id_;
    buf[1] = 0xA0;
    buf[2] = channel_.tx.first().id;
    buf[3] = 0xFF;

    len = channel_.write(buf + 4);

    uint8_t ex_len = len + 4;
    uint16_t data_len = 0x8000 + ex_len;

    uint8_t check_sum;

    for (int i = 0; i < ex_len; i++) {
      check_sum ^= buf[i];
    }

    serial_.write(0xA5);
    serial_.write(0x5A);
    serial_.write(data_len >> 8);
    serial_.write(data_len & 0xFF);
    serial_.write(buf, ex_len);
    serial_.write(check_sum);
  }
}

void TWELITE::receive() {
  while (serial_.available() > 0) {
    rx_buf_[rx_count_] = serial_.read();
    Serial.printf("%02X", rx_buf_[rx_count_]);
    rx_count_++;

    if (rx_count_ >= TWE_BUF_SIZE) {
      error();
      rx_count_ = 0;
      Serial.println("OVERFLOW");
    }

    if (rx_count_ < 4) continue;

    if (rx_buf_[0] != 0xA5 || rx_buf_[1] != 0x5A) {
      error();
      rx_count_ = 0;
      Serial.println("INVALID");
      continue;
    }

    uint16_t ex_len = (((uint16_t)rx_buf_[2] & 0x7F) << 8) + rx_buf_[3];

    if (rx_count_ < 6 + ex_len) continue;

    uint8_t buf[TWE_BUF_SIZE];

    uint8_t check_sum = 0;

    for (int i = 0; i < ex_len; i++) {
      buf[i] = rx_buf_[4 + i];
      check_sum ^= buf[i];
    }

    if (check_sum != rx_buf_[4 + ex_len] || rx_buf_[5 + ex_len] != 0x04) {
      error();
      Serial.printf("CHECKSUM %X != %X\n", check_sum, rx_buf_[4 + ex_len]);
    }
    else if (buf[0] == 0xDB) { // ACK message
      uint8_t ack_id = buf[2];
      bool ok = buf[3] == 1;
      if (buf[1] != 0xA1 || !ok) error();
      else sent_count_++;

      Serial.printf("ACK %d\n", ok);
    }
    else if (buf[0] <= 0x64 || buf[0] == 0x78) { // Receive
      if (buf[1] != 0xA0) {
        error();
        Serial.println("ERROR");
      }
      else {
        uint8_t ack_id = buf[2];
        lqi_ = buf[10];
        unsigned len = ((uint16_t)buf[11] << 8) + buf[12];

        channel_.read(buf + 14, len);
        callback_(channel_.rx);

        received_count_++;
      }
    }
    else {
      error();
    }

    rx_count_ = 0;
  }
}

void TWELITE::listen(MessageCallback func) {
  callback_ = func;
}
