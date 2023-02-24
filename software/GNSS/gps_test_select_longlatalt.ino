#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <time.h>
SoftwareSerial serial(13,12);

TinyGPSPlus gps;

void GpsTimeRead(void);
void convert(void);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  serial.begin(9600);

  Serial.println("GPS start!");

}

void loop() {
  // put your main code here, to run repeatedly:
  while(serial.available() > 0){
    char c = serial.read();
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
      Serial.println(gps.satellites.isValid());
      GpsTimeRead();
      convert();
      Serial.println(gps.altitude.meters(), 6);
      Serial.println("");
    }
  }
}
void convert(){
  long scale=10000000UL;
  long lat = gps.location.rawLat().deg*scale+gps.location.rawLat().billionths/100UL;
  if(gps.location.rawLat().negative) lat=-lat;
  long lon = gps.location.rawLng().deg*scale+gps.location.rawLng().billionths/100UL;
  if(gps.location.rawLng().negative) lon=-lon;
  Serial.println(lat);
  Serial.println(lon);
}

void  GpsTimeRead(void){
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
}
