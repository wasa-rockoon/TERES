#include <TaskScheduler.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <time.h>

#include "RP2040Module.h"

#define GPS_BAUD 9600

#define STATUS_FREQ 1
#define GPS_LOST_AGE 1000

Scheduler scheduler;

RP2040Module module;

SoftwareSerial gnss_serial(13,12);
TinyGPSPlus gps;

void status();
Task task_status(TASK_SECOND / STATUS_FREQ,
                 TASK_FOREVER, &status, &scheduler, false);



/* void blink(); */

/* Task task_blink(1000 * TASK_MILLISECOND, TASK_FOREVER, &blink, &scheduler, true); */

void setup() {
  // put your setup code here, to run once:

  bool ok = module.begin();

  gnss_serial.begin(GPS_BAUD);

  delay(1000);

  task_status.enable();

  module.indicator.clearError();
}

/* void blink() { */
/*   module.indicator.blink(); */

/*   Message test('T', 0, 2); */
/*   test.entries[0].set('I', (int32_t)123); */
/*   test.entries[1].set('F', (float)56.7); */

/*   module.bus.send(test); */

/*   Serial.print("ok: "); */
/*   Serial.print(module.bus.getMessageCount()); */
/*   Serial.print(", error: "); */
/*   Serial.println(module.bus.getErrorCount()); */
/* } */

void status() {
  if (!gps.location.isValid() || gps.location.age() > GPS_LOST_AGE) {
    module.indicator.errorEvent(50);
    Serial.println("No GPS");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.execute();

  while(gnss_serial.available() > 0){
    char c = gnss_serial.read();
    gps.encode(c);
    if(gps.location.isUpdated()){

      //Serial.print("Lat=\t");   Serial.print(gps.location.lat(), 6);
      //Serial.print(" Lng=\t");   Serial.print(gps.location.lng(), 6);

      //Serial.print("Time=\t");   Serial.print(gps.time.year());
      //Serial.print("/");   Serial.print(gps.time.month());
      //Serial.print("/");   Serial.print(gps.time.day());
      //Serial.print(" ");   Serial.print(gps.time.hour());
      //Serial.print(":");   Serial.print(gps.time.minute());
      //Serial.print(":");   Serial.print(gps.time.second());
      //Serial.println("");
      Serial.println(gps.satellites.value());

      tm t;
      time_t tim;
      tm* ltim;

      if( gps.time.isValid()){
        t.tm_year = gps.date.year() - 1900;
        t.tm_mon = gps.date.month() - 1; //0からカウントするので-1
        t.tm_mday = gps.date.day();
        t.tm_hour = gps.time.hour(); //日本：世界標準時から9時間ずれ
        t.tm_min = gps.time.minute();
        t.tm_sec = gps.time.second()+1;
        t.tm_isdst= -1;

        tim = mktime(&t);
        Serial.println(tim);
      }

      long scale=10000000UL;
      long lat = gps.location.rawLat().deg * scale
        + gps.location.rawLat().billionths/100UL;
      if(gps.location.rawLat().negative) lat=-lat;
      long lon = gps.location.rawLng().deg * scale
        + gps.location.rawLng().billionths/100UL;
      if(gps.location.rawLng().negative) lon=-lon;


      Serial.println(lat);
      Serial.println(lon);

      Serial.println(gps.altitude.meters(), 6);
      Serial.println("");


      Message tlm('P', 0, 5);
      tlm.entries[0].set('A', (uint32_t)lat);
      tlm.entries[1].set('O', (uint32_t)lon);
      tlm.entries[2].set('H', (float)gps.altitude.meters());
      tlm.entries[3].set('T', (uint32_t)tim);
      tlm.entries[4].set('S', (uint32_t)gps.satellites.value());
      module.bus.send(tlm);

      module.indicator.blink(50);
    }
  }
}

