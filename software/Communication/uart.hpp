/*
 * uart.hpp
 *
 *  Created on: Aug 19, 2022
 *      Author: yuuki.fj
 */

#ifndef INC_UART_HPP_
#define INC_UART_HPP_

#include "Arduino.h"

class UART {
public:
	UART(HardwareSerial& serial, uint8_t* buf, unsigned ring_size, unsigned tail_size);

	bool begin();

	bool rxIsEmpty() {
	  return available() == 0;
	}
	unsigned available();

	inline bool write(const uint8_t* buf, unsigned len) {
    serial_.write(buf, len);
    return true;
	}
	inline bool write(uint8_t c) {
     serial_.write(c);
     return true;
	}

	int16_t read();
	int16_t peek(unsigned pos = 0);
	uint8_t* read(unsigned len);

	void clear();

	void changeBaudRate(unsigned baud);


protected:
	HardwareSerial& serial_;

	uint8_t* buf_;
	unsigned ring_size_;
	unsigned tail_size_;

	unsigned rx_read_ptr_;
	unsigned rx_write_ptr_;
};



#endif /* INC_UART_HPP_ */
