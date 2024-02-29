#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <time.h>
#include "RX8025_RTC.h"

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

#define SECOND_ADJUSTMENT 6

#define SDCARD_MOSI_PIN SPI0_MOSI
#define SDCARD_MISO_PIN SPI0_MISO
#define SDCARD_SS_PIN SPI0_CS
#define SDCARD_SCK_PIN SPI0_SCK

File file;
int i;
char file_name[20];

RX8025_RTC rtc;

void TimeRead(void);

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

//コンパイル時刻にセットの場合、以下をコメント解除
  //initDateTime();
  //rtc.write(tm);

  SPI.setTX(SPI0_MOSI);
  SPI.setRX(SPI0_MISO);
  SPI.setSCK(SPI0_SCK);

  delay(1000);

  digitalWrite(LED_ERROR, LOW);

  Serial.println("init SD card");

  delay(5000);

   //CSPINはSDカードのcsを接続した端子番号
  if (!SD.begin(SPI0_CS)) {
    Serial.println("device error");
    return;
  }

  tmElements_t tm2;
  tm2 = rtc.read();
  sprintf(file_name,"%d%d_%d_%d.txt",tm2.Month,tm2.Day,tm2.Hour,tm2.Minute);
  
  Serial.println("create new file");

  file = SD.open(file_name, FILE_WRITE);
  // if the file opened okay, write to it:
  if (file) {
    Serial.println("Writing to");
    }else{
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
  Serial.print(i);
  Serial.print(" ");
  
  tmElements_t tm3;
  tm3 = rtc.read();
  char s[20];
  sprintf(s,"%d/%d/%d %d:%d:%d",tmYearToCalendar(tm3.Year),tm3.Month,tm3.Day,tm3.Hour,tm3.Minute,tm3.Second);
  Serial.println(s);
  
if (file) {
  if(i<=60){
    TimeRead();
  }else{
// close the file:
    file.close();
    Serial.println("write done.");
  }
}else {
    // if the file didn't open, print an error:
    Serial.println("error creating test.txt");
}
  i++;
}

void  TimeRead(void){
  tm t;
  time_t tim;
  tm* ltim;
  
  tmElements_t tm3;
  tm3 = rtc.read();

    t.tm_year = tmYearToCalendar(tm3.Year) - 1900;
    t.tm_mon = tm3.Month - 1; //0からカウントするので-1
    t.tm_mday = tm3.Day;
    t.tm_hour = tm3.Hour+3; //日本：世界標準時から9時間ずれ
    t.tm_min = tm3.Minute;
    t.tm_sec = tm3.Second+1;
    t.tm_isdst= -1;

    tim = mktime(&t);
    file.println(tim);  
  
}

/*bool initDateTime(){
  const char* monthNames[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  char mon[12];
  int Year, Month, Day, Hour, Minute, Second;

  if(sscanf(__DATE__,"%s %d %d",mon, &Day, &Year) != 3){
    return false;
  }
  if(sscanf(__TIME__,"%d:%d:%d",&Hour, &Minute, &Second) != 3){
    return false;
  }

  uint8_t idx;
  Month = 0;
  for(idx = 0; idx < 12; idx++){
    if(strcmp(mon, monthNames[idx]) == 0){
      Month = idx + 1;
      break;
    }
  }
  if(Month == 0){
      return false;
  }
  Second += SECOND_ADJUSTMENT;
  tm.Year = CalendarYrToTm(Year);
  tm.Month = Month;
  tm.Day = Day;
  tm.Hour = Hour;
  tm.Minute = Minute;
  tm.Second = Second;
  return true;
}*/
