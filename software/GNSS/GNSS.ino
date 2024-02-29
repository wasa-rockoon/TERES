#include <TaskScheduler.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <time.h>
#include <Packet.hpp>

#include "RP2040Module.h"

#define GPS_BAUD 9600

#define STATUS_FREQ 1
#define GPS_LOST_AGE 10000

Scheduler scheduler;

RP2040Module module('P');

SoftwareSerial gnss_serial(13, 12);
TinyGPSPlus gps;

bool gps_alive = false;
bool gps_ok = false;

void status();
Task task_status(TASK_SECOND / STATUS_FREQ,
                 TASK_FOREVER, &status, &scheduler, false);

void setup() {
  // put your setup code here, to run once:

  bool ok = module.begin();

  gnss_serial.begin(GPS_BAUD);

  delay(1000);

  task_status.enable();

  module.indicator.clearError();
}

void status() {
  if (!gps.location.isValid() || gps.location.age() > GPS_LOST_AGE) {
    gps_ok = false;;
    module.indicator.errorEvent(100);
    Serial.println("No GPS");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler.execute();

  while (gnss_serial.available() > 0) {
    gps_alive = true;

    char c = gnss_serial.read();
    gps.encode(c);
    if (gps.location.isUpdated()) {
      gps_ok = true;

      tm t;
      time_t tim;

      t.tm_year = gps.date.year() - 1900;
      t.tm_mon = gps.date.month() - 1;
      t.tm_mday = gps.date.day();
      t.tm_hour = gps.time.hour();
      t.tm_min = gps.time.minute();
      t.tm_sec = gps.time.second() + 1;
      t.tm_isdst = -1;

      tim = mktime(&t);

      long scale = 10000000UL;
      int32_t lat = gps.location.rawLat().deg * scale
                    + gps.location.rawLat().billionths / 100UL;
      if (gps.location.rawLat().negative) lat = -lat;
      int32_t lon = gps.location.rawLng().deg * scale
                    + gps.location.rawLng().billionths / 100UL;
      if (gps.location.rawLng().negative) lon = -lon;

      Serial.println(lat);
      Serial.println(lon);

      Serial.println(gps.altitude.meters(), 6);
      Serial.println("");

      uint8_t buf[BUF_SIZE(5)];
      Packet tlm(buf, sizeof(buf));
      tlm.set(TELEMETRY, 'P');
      tlm.begin()
        .append('A', lat)
        .append('O', lon)
        .append('H', (float)gps.altitude.meters())
        .append('T', (uint32_t)tim)
        .append('S', (uint8_t)gps.satellites.value());
      module.bus.send(tlm);

      module.indicator.blink(50);
    }
  }

  module.bus.sanity(1, gps_alive);
  module.bus.sanity(2, gps_ok);
}

void loop1() {
  module.update();
}
