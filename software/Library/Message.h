#pragma once

#include <Arduino.h>

#ifndef MAX_ENTRIES
#define MAX_ENTRIES 16
#endif

#define BIN_TX_QUEUE_SIZE 8
#define HEX_TX_QUEUE_SIZE 8
#define CAN_TX_QUEUE_SIZE 8

union Payload {
	uint8_t bytes[4];
	uint32_t uint;
	int32_t int_;
	float float_;

//	template <>
//	inline uint32_t as<uint32_t>() { return uint; };
//	template <>
//	inline int32_t Payload::as<int32_t>() { return int_; };

};

template <typename T>
inline T payloadAs(Payload p);

template <>
inline float payloadAs<float>(Payload p) { return p.float_; };

const union Payload default_payload = { .uint = 0 };

struct Entry {
  uint8_t type;
  union Payload payload;

  void set(uint8_t type);
  void set(uint8_t type, const uint8_t* bytes);
  void set(uint8_t type, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
  void set(uint8_t type, int32_t value);
  void set(uint8_t type, uint32_t value);
  void set(uint8_t type, float value);

  uint8_t encode(uint8_t* buf) const;
  uint8_t encodeHex(uint8_t* buf) const;
	uint8_t decode(const uint8_t* buf);
	uint8_t decodeHex(const uint8_t* buf);

	Entry& operator=(const Entry& entry);
};

struct Message {
	uint8_t id;
	uint8_t to;
	uint8_t from;
	uint8_t size;
	Entry entries[MAX_ENTRIES];

	Message();
	Message(uint8_t id, uint8_t to, uint8_t from, uint8_t size);

	void setHeader(Entry header);
	Entry getHeader() const;

	float get(uint8_t type, uint8_t index, float default_) const;
	bool get(uint8_t type, uint8_t index = 0) const;
	bool get(uint8_t type, uint8_t index, union Payload& p) const;

	void addTimestamp(uint32_t time);

	Message& operator=(const Message& message);
};

template<uint8_t N>
class Queue {
public:
	Queue();

	inline uint8_t size() const { return size_; };
  inline bool isEmpty() const { return size_ == 0; };
	inline Message& first()  { return buf[read_ptr_]; };
	inline Message& last() { return buf[(write_ptr_ - 1) % N]; };

//	bool push(Message& message);
//	bool push();
//	bool pop(Message& message);
//	bool pop();
//
	bool push(Message& message) {
		if (size_ == N) return false;

		buf[write_ptr_] = message;

		return push();
	}
	bool push() {
		if (size_ == N) return false;

		write_ptr_++;
		if (write_ptr_ >= N) write_ptr_ = 0;
		size_++;

		return true;
	}
	bool pop(Message& message) {
		if (size_ == 0) return false;

		message = first();

		return pop();
	}
	bool pop() {
		if (size_ == 0) return false;

		read_ptr_++;
		if (read_ptr_ >= N) read_ptr_ = 0;
		size_--;

		return true;
	}

private:
	Message buf[N];
	uint8_t size_;
	uint8_t read_ptr_;
	uint8_t write_ptr_;
};


template<uint8_t TXQ>
class Channel {
public:
	Queue<TXQ> tx;
	Message rx;

	Channel();

  inline unsigned getErrorCount() { return error_count_; };

private:
  unsigned error_count_;

  inline void error() { error_count_++ };
};

class BinaryChannel: public Channel<BIN_TX_QUEUE_SIZE> {
public:
  BinaryChannel();

  bool read(const uint8_t* data, uint8_t len);

	unsigned write(uint8_t* data);

private:
	uint8_t rx_buf_[6];
	uint8_t rx_buf_count_;
  uint8_t rx_buf_max_;
};
