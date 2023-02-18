#include <TaskScheduler.h>
#include <Bus.h>
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

#define SDCARD_MOSI_PIN SPI0_MOSI
#define SDCARD_MISO_PIN SPI0_MISO
#define SDCARD_SS_PIN SPI0_CS
#define SDCARD_SCK_PIN SPI0_SCK

#include <SD.h>


Scheduler scheduler;


File file;
int i;
char file_name[20]="test.txt";

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);

  pinMode(SD_INSERTED, INPUT_PULLUP);

  pinMode(SPI0_CS, OUTPUT);

  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_ERROR, HIGH);

  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();

  SPI.setTX(SPI0_MOSI);
  SPI.setRX(SPI0_MISO);
  SPI.setSCK(SPI0_SCK);

  Bus.begin(Serial1, Serial2);

  delay(1000);

  digitalWrite(LED_ERROR, LOW);

  Serial.println("init SD card");

  delay(5000);

   //CSPINはSDカードのcsを接続した端子番号
  if (!SD.begin(SPI0_CS)) {
    Serial.println("device error");
    return;
  }
  Serial.println("create new file");

  file = SD.open(file_name, FILE_WRITE);
  // if the file opened okay, write to it:
  if (file) {
    Serial.println("Writing to");
    file.println("testing 1, 2, 3.");
    file.println("success!");
    for(i=0;i<50;i++){
      file.print(i);  file.print(',');
      file.println(i+1);
    }
// close the file:
    file.close();
    Serial.println("write done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error creating test.txt");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_STATUS, HIGH);
  delay(500);
  digitalWrite(LED_STATUS, LOW);
  delay(500);

  i2c_scanner();
}
