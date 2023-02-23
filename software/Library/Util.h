
#include <Arduino.h>


void printlnBytes(const uint8_t* bytes, unsigned len, unsigned split = 0) {
  Serial.print("0x");
  for (int n = 0; n < len; n++) {
    if (split > 0 && n % split == 0) Serial.print('_');
    if (bytes[n] < 16) Serial.print("0");
    Serial.print(bytes[n], HEX);
  }
  Serial.println();
}
