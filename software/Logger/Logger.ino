#include <TaskScheduler.h>
#include <PacketSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <TimeLib.h>
#include <RX8025_RTC.h>

#include "RP2040Module.h"

#define DEBUG

// #define SET_RTC_TIME
#define SECOND_ADJUSTMENT 6

// Pins
#define I2C0_SDA_PIN 8
#define I2C0_SCL_PIN 9
#define SPI0_SCK_PIN 18
#define SPI0_MOSI_PIN 19
#define SPI0_MISO_PIN 20
#define SPI0_CS_PIN 21
#define SD_INSERTED_PIN 11

#define SDCARD_MOSI_PIN SPI0_MOSI_PIN
#define SDCARD_MISO_PIN SPI0_MISO_PIN
#define SDCARD_SS_PIN SPI0_CS_PIN
#define SDCARD_SCK_PIN SPI0_SCK_PIN

#include <SD.h>

// Constants
#define SEND_TIME_FREQ 1


// Tasks
void send_time();
Task task_send_time(1000 / SEND_TIME_FREQ * TASK_MILLISECOND, TASK_FOREVER,
                    &send_time, &scheduler, false);

// Objects

RX8025_RTC rtc;

RP2040Module module;

BinaryChannel log_channel;

// Variables

File file;
bool sd_ok = false;
unsigned wrote_count = 0;
unsigned dropped_count = 0;


void setup() {
  // put your setup code here, to run once:

  pinMode(SD_INSERTED_PIN, INPUT_PULLUP);

  pinMode(SPI0_CS_PIN, OUTPUT);


  SPI.setTX(SPI0_MOSI_PIN);
  SPI.setRX(SPI0_MISO_PIN);
  SPI.setSCK(SPI0_SCK_PIN);

  // SD card

  if (digitalRead(SD_INSERTED_PIN)) {
    Serial.println("SD card is not inserted.");
  }

  delay(100);

  Wire.setSDA(I2C0_SDA_PIN);
  Wire.setSCL(I2C0_SCL_PIN);
  Wire.begin();

  // RTC

#ifdef SET_RTC_TIME
  tmElements_t tm;
  initDateTime(tm);
  rtc.write(tm);
#endif

  task_send_time.enable();

  bool ok = module.begin();
}

bool sd_init() {

  if (digitalRead(SD_INSERTED_PIN)) {
    delay(1000);
    return false;
  }

  delay(100);

  Serial.println("SD card init.");

  if (!SD.begin(SDCARD_SS_PIN)) {
    Serial.println("Device error.");
    delay(1000);
    return false;
  }

  tmElements_t tm;
  tm = rtc.read();

  char dir_name[16];
  char file_name[32];

  sprintf(
    dir_name, "%d%02d%02d",
    tmYearToCalendar(tm.Year),tm.Month,tm.Day);
  sprintf(
    file_name, "%s/%02d%02d%2d.bin",
    dir_name,tm.Hour,tm.Minute,tm.Second);

  if (SD.exists(file_name)) {
    Serial.printf("%s already exists.", file_name);
    delay(1000);
    return false;
  }

  if (!SD.exists(dir_name)) SD.mkdir(dir_name);
  file = SD.open(file_name, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to create file.");
    delay(1000);
    return false;
  }

  Serial.printf("Created file: %s\n", file_name);

  module.bus.listen(write_log);

  return true;
}

void send_time() {
  tmElements_t tm = rtc.read();
  time_t t = makeTime(tm);

  char s[32];
  sprintf(s, "%d/%02d/%02d %02d:%02d:%02d",
          tmYearToCalendar(tm.Year), tm.Month, tm.Day,
          tm.Hour, tm.Minute, tm.Second);
  Serial.println(s);

  Message message('l', 0, 3);
  message.entries[0].set('t', (int32_t)t);
  message.entries[1].set('w', (uint32_t)wrote_count);
  message.entries[2].set('d', (uint32_t)dropped_count);

  module.bus.send(message);

  if (!sd_ok) return;

  write_log(message);

  file.flush();

  Serial.printf("Wrote %d, Dropped %d.\n", wrote_count, dropped_count);
}

void write_log(const Message& message) {
  if (!sd_ok) return;

  Message log_message = message;
  log_message.addTimestamp(millis());

  if (log_channel.tx.isFull()) {
    log_channel.tx.pop();
    dropped_count++;
    module.indicator.errorEvent(10);
  }
  log_channel.tx.push(log_message);

  uint8_t buf[BUFFER_SIZE];
  unsigned len;

  while ((len = log_channel.nextWriteSize()) > 0) {
    /* Serial.println(file.availableForWrite()); */
    if (!file.availableForWrite()) {
      return;
    }

    len = log_channel.write(buf);

    uint8_t encoded[BUFFER_SIZE + 2];
    unsigned encoded_len = COBS::encode(buf, len, encoded);
    encoded[encoded_len] = 0;

    file.write(encoded, encoded_len + 1);

    wrote_count++;
  }

  module.indicator.blink(1);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (sd_ok) {
    if (digitalRead(SD_INSERTED_PIN)) {
      Serial.println("SD card removed.");
      sd_ok = false;
      module.indicator.setError(ErrorStatus::Error);
      return;
    }
  }
  else {
    sd_ok = sd_init();
    if (sd_ok) module.indicator.clearError();
  }

  scheduler.execute();
}

bool initDateTime(tmElements_t& tm) {
  const char* monthNames[] = 
    {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
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
}
