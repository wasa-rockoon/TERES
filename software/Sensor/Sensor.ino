#include <I2C_Scanner.h>
#include <Wire.h>

#define LED_STATUS 0
#define LED_ERROR 25
#define I2C1_SDA 14
#define I2C1_SCL 15


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);

  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_ERROR, HIGH);

  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire1.begin();

  delay(1000);

  digitalWrite(LED_ERROR, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_STATUS, HIGH);
  delay(500);
  digitalWrite(LED_STATUS, LOW);
  delay(500);

  i2c_scanner(Wire1);
}
