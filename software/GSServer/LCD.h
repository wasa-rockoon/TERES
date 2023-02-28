
#pragma once

#include <Arduino.h>
#include "Wire.h"

#define LCD_ADRS 0x3E
#define LINE_WIDTH 16

class LCD {
public:
    char line1[LINE_WIDTH + 1];
    char line2[LINE_WIDTH + 1];

    bool begin();
    void show();
    void update();

private:
    int update_sequence;
    unsigned long update_wait_millis;

    void writeData(byte t_data);
    void writeCommand(byte t_command);
};
