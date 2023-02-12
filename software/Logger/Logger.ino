#include <I2C_Scanner.h>
#include <SPI.h>
#include <Wire.h>

// Pins
#define LED_STATUS 0
#define LED_ERROR 25
#define SWITCH 23
#define I2C0_SDA 8
#define I2C0_SCL 9
#define SPI0_SCK 18
#define SPI0_MOSI 19
#define SPI0_MISO 20
#define SPI0_CS 21
#define SD_INSERTED 11

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);

  pinMode(SD_INSERTED, INPUT_PULLUP);

  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_ERROR, HIGH);

  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();

  SPI.setTX(SPI0_MOSI);
  SPI.setRX(SPI0_MISO);
  SPI.setSCK(SPI0_SCK);

  delay(1000);

  digitalWrite(LED_ERROR, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_STATUS, HIGH);
  delay(500);
  digitalWrite(LED_STATUS, LOW);
  delay(500);

  i2c_scanner();
}
