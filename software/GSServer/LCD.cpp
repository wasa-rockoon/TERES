
#include "LCD.h"

bool LCD::begin() {
    update_sequence = -1;

    delay(30);
    writeCommand(0x38);
    delay(30);
    writeCommand(0x39);
    delay(30);
    writeCommand(0x14);
    delay(30);
    writeCommand(0x73);
    delay(30);
    writeCommand(0x56);
    delay(30);
    writeCommand(0x6C);
    delay(30);
    writeCommand(0x38);
    delay(30);
    writeCommand(0x01);
    delay(30);
    writeCommand(0x0C);
    delay(30);

    return true;
}

void LCD::show() {
    update_sequence = 0;
    update_wait_millis = millis();
}

void LCD::update() {
    if (millis() < update_wait_millis) return;

    if (update_sequence == 0) {
        writeCommand(0x80);
    }
    else if (update_sequence <= LINE_WIDTH) {
        writeData(line1[update_sequence - 1]);
    }
    else if (update_sequence == LINE_WIDTH + 1) {
        writeCommand(0x40+0x80); // 2LINE TOP
    }
    else if (update_sequence <= 2 * LINE_WIDTH + 1) {
        writeData(line2[update_sequence - LINE_WIDTH - 2]);
    }
    else return;

    update_sequence++;
}

void LCD::writeData(byte t_data) {
    Wire.beginTransmission(LCD_ADRS);
    Wire.write(0x40);//送信する１バイトのデータ
    Wire.write(t_data);//文字列の送信
    Wire.endTransmission();
    update_wait_millis = millis() + 1;
}

void LCD::writeCommand(byte t_command) {
    Wire.beginTransmission(LCD_ADRS);
    Wire.write(0x00);
    Wire.write(t_command);
    Wire.endTransmission();
    update_wait_millis = millis() + 10;
}
