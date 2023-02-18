
#include "Message.h"


//template <>
//inline uint32_t Payload::as<uint32_t>() { return uint; };
//template <>
//inline int32_t Payload::as<int32_t>() { return int_; };
//template <>
//inline float Payload::as<float>() { return float_; };


void Entry::set(uint8_t type) {
	this->type = type, payload.uint = 0;
}

void Entry::set(uint8_t type, const uint8_t* bytes) {
	this->type = type;
	for (int i = 0; i < 4; i++) payload.bytes[i] = bytes[i];
}
void Entry::set(uint8_t type, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
	this->type = type;
	payload.bytes[0] = byte0;
	payload.bytes[1] = byte1;
	payload.bytes[2] = byte2;
	payload.bytes[3] = byte3;
}

void Entry::set(uint8_t type, int32_t value) {
	this->type = type;
	payload.int_ = value;
}

void Entry::set(uint8_t type, uint32_t value) {
	this->type = type;
	payload.uint = value;
}

void Entry::set(uint8_t type, float value) {
	this->type = type;
	payload.float_ = value;
}

uint8_t Entry::encode(uint8_t* buf) const {
    if (payload.int_ == 0) {
    	buf[0] = type | 0b10000000;
    	return 1;
    }
    else {
    	buf[0] = type;
        for (int i = 0; i < 4; i++) buf[i + 1] = payload.bytes[i];
        return 5;
    }
}

uint8_t Entry::decode(const uint8_t* buf) {
	type = buf[0] & 0b01111111;

	if ((buf[0] & 0b10000000)) {
		payload.uint = 0;
		return 1;
	}
	else {
		for (int i = 0; i < 4; i++) payload.bytes[i] = buf[1 + i];
		return 5;
	}
}

uint8_t Entry::encodeHex(uint8_t* buf) const {
    uint8_t data[5];
    uint8_t len;

    if (payload.int_ == 0) {
        data[0] = type | 0b10000000;
        len = 1;
    }
    else {
    	data[0] = type;
        for (int i = 0; i < 4; i++) data[i + 1] = payload.bytes[i];
    	len = 5;
    }

    for (int i = 0; i < len; i++) {
        char hex[3];
        sprintf(hex, "%X", data[i]);
        if (data[i] < 16) {
            buf[i*2]   = '0';
            buf[i*2+1] = hex[0];
        }
        else {
            buf[i*2]   = hex[0];
            buf[i*2+1] = hex[1];
        }
    }
    buf[len * 2] = '\0';
    return len * 2;
}

uint8_t Entry::decodeHex(const uint8_t* buf) {
    char *err;
    uint8_t data[5];
    char hex[3];

    hex[0] = buf[0];
    hex[1] = buf[1];
    hex[2] = '\0';
    data[0] = (uint8_t)strtol(hex, &err, 16);
    if (*err != '\0') return 0;

    type = data[0] & 0b01111111;

    if (data[0] & 0b10000000) {
    	payload.uint = 0;
    	return 2;
    }
    else {
		for (int i = 1; i < 5; i++) {
			hex[0] = buf[2 * i];
			hex[1] = buf[2 * i + 1];
			hex[2] = '\0';
			data[i] = (uint8_t)strtol(hex, &err, 16);
		}
		if (*err != '\0') return false;
		for (int i = 0; i < 4; i++) payload.bytes[i] = data[1 + i];
		return 10;
    }
}

Entry& Entry::operator=(const Entry& entry) {
	type = entry.type;
	payload.uint = entry.payload.uint;
	return *this;
}


Message::Message(): id(0), to(0), from(0), size(0) {};
Message::Message(uint8_t id, uint8_t to, uint8_t from, uint8_t size):
		id(id), to(to), from(from), size(size) {};



void Message::setHeader(Entry header) {
	id = header.payload.bytes[0];
	to = header.payload.bytes[1];
	from = header.payload.bytes[2];
	size = header.payload.bytes[3];
}

Entry Message::getHeader() const {
	Entry header;
	header.type = 0;
	header.payload.bytes[0] = id;
	header.payload.bytes[1] = to;
	header.payload.bytes[2] = from;
	header.payload.bytes[3] = size;
	return header;
}

float Message::get(uint8_t type, uint8_t index, float dufault) const {
	Payload p = { .float_ = 0.0 };
	get(type, index, p);
	return p.float_;
}

bool Message::get(uint8_t type, uint8_t index) const {
	Payload p;
	return get(type, index, p);
}

bool Message::get(uint8_t type, uint8_t index, union Payload& p) const {
	for (int n = 0; n < size; n++) {
		if (entries[n].type == type) {
			if (index == 0) {
				p = entries[n].payload;
				return true;
			}
			else index--;
		}
	}
	return false;
}

void Message::addTimestamp(uint32_t time) {
	if (size >= MAX_ENTRIES) {
		return;
	}
	entries[size].set('t', time);
	size++;
}

Message& Message::operator=(const Message& message) {
	id = message.id;
	to = message.to;
	from = message.from;
	size = message.size;
	for (int n = 0; n < size; n++) {
		entries[n] = message.entries[n];
	}
	return *this;
}

template<uint8_t N>
Queue<N>::Queue() {
	size_ = 0;
	read_ptr_ = 0;
	write_ptr_ = 0;
}

//template<uint8_t N>
//bool Queue<N>::push(Message& message) {
//	if (size_ == N) return false;
//
//	buf[write_ptr_] = message;
//
//	return push();
//}
//
//template<uint8_t N>
//bool Queue<N>::push() {
//	if (size_ == N) return false;
//
//	write_ptr_++;
//	if (write_ptr_ >= N) write_ptr_ = 0;
//	size_++;
//
//	return true;
//}
//
//template<uint8_t N>
//bool Queue<N>::pop(Message& message) {
//	if (size_ == 0) return false;
//
//	message = first();
//
//	return pop();
//}
//
//template<uint8_t N>
//bool Queue<N>::pop() {
//	if (size_ == 0) return false;
//
//	read_ptr_++;
//	if (read_ptr_ >= N) read_ptr_ = 0;
//	size_--;
//
//	return true;
//}

template<uint8_t TXQ>
Channel<TXQ>::Channel() {
  error_count_ = 0;
}


BinaryChannel::BinaryChannel(): Channel() {}


bool BinaryChannel::read(const uint8_t* data, uint8_t len) {
  unsigned i = 0;
  Entry header;
  i += header.decode(data);

  rx.setHeader(header);

  for (uint8_t n = 0; n < header.size; n++) {
    if (i > len) {
      error();
      return false;
    }
    Entry entry;
    i += entry.decode(data + i);
    rx.entries[n] = entry;
  }
  return true;
}


unsigned BinaryChannel::write(uint8_t* data) {
  if (tx.size() == 0) {
		return 0;
	}
  unsigned i = 0;

	Entry header = tx.first().getHeader();
	i += header.encode(data);
  for (uint8_t n = 0; n < header.size; n++) {
		i += tx.first().entries[n].encode(data + i);
  }

	tx.pop();

	return i;
}

// bool BinaryChannel::receive(uint8_t data) {
//   rx_buf_[rx_buf_count_] = data;
//   if (rx_buf_count_ == 0) {
//     Entry entry;
//     rx_buf_max_ = entry.decode(rx_buf_);
//   }

//   rx_buf_count_++;

//   if (rx_buf_count_ >= rx_buf_max_) {
//     Entry entry;
//     entry.decode(rx_buf_);

//     if (entry.type == 0) {
//       rx.setHeader(entry);

//       if (rx.size > MAX_ENTRIES) rx.size = MAX_ENTRIES;

//       receiving_ = 0;
//     }
//     else if (receiving_ != -1){
//       rx.entries[receiving_] = entry;
//       receiving_++;
//     }

//     if (receiving_ >= rx.size) {
//       receiving_ = -1;
//       return true;
//     }

//     rx_buf_count_ = 0;
//   }

//   return false;
// }


// uint8_t BinaryChannel::send(uint8_t* data, uint8_t& len) {
//   if (tx.size() == 0) {
// 		len = 0;
// 		return 0;
// 	}

// 	if (sending_ == -1) {
// 		Entry header = tx.first().getHeader();
// 		len = header.encode(data);
// 		sending_ = 0;
// 	}
// 	else {
// 		len = tx.first().entries[sending_].encode(data);
// 		sending_++;
// 	}

// 	int remains = tx.first().size - sending_;
// 	if (remains == 0) {
// 		sending_ = -1;

// 		tx.pop();
// 	}

// 	return tx.size();
// }


