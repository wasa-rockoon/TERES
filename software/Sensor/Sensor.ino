
#include <TaskScheduler.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>

#include "RP2040Module.h"
#include "Shared.hpp"
#include "Datatypes.h"


#define SEND_ATT_FREQ 100
#define SEND_ALT_FREQ 10

#define QNH_HPA (1013.25)

#define ASCENT_RATE_FILTER_A 0.9


#define I2C1_SDA 14
#define I2C1_SCL 15
#define BNO055_ADDR 0x28
#define BNE280_ADDR 0x76

// Tasks

Scheduler scheduler;

void send_attitude();
Task task_send_attitude(TASK_SECOND / SEND_ATT_FREQ,
                        TASK_FOREVER, &send_attitude, &scheduler, false);

void send_altitude();
Task task_send_altitude(TASK_SECOND / SEND_ALT_FREQ,
                        TASK_FOREVER, &send_altitude, &scheduler, false);

// Objects

RP2040Module module('S');

Adafruit_BNO055 bno(55, BNO055_ADDR, &Wire1);
Adafruit_BME280 bme;

// Variables
Shared<FlightMode> mode(FlightMode::STANDBY);
FlightSequence sequence = FlightSequence::BEFORE_FLIGHT;

float pressure_altitude = 0.0;
float ascent_rate = 0.0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  bool module_ok = module.begin();

  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire1.begin();

  bool bno_ok = bno.begin();
  bool bme_ok = bme.begin(BNE280_ADDR, &Wire1);

  delay(500);

  if (bno_ok) task_send_attitude.enable();
  if (bme_ok) task_send_altitude.enable();

  module.bus.subscribe(mode, TELEMETRY, 'B', 'm');

  module.bus.sanity(1, bno_ok);
  module.bus.sanity(2, bme_ok);

  if (module_ok && bno_ok && bme_ok) {
    module.indicator.clearError();
  }
}

void send_attitude() {

  uint8_t buf[BUF_SIZE(7)]; 
  Packet attitude(buf, sizeof(buf));
  attitude.set(TELEMETRY, 'A', BROADCAST);

  sensors_event_t event;
  bno.getEvent(&event);
  imu::Quaternion q = bno.getQuat();
  attitude.end()
    .append('Q', (float)q.x())
    .append('Q', (float)q.y())
    .append('Q', (float)q.z())
    .append('Q', (float)q.w());

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  attitude.end()
    .append('A', (float)event.acceleration.x)
    .append('A', (float)event.acceleration.y)
    .append('A', (float)event.acceleration.z);

  float accel = std::sqrt(event.acceleration.x * event.acceleration.x 
                         + event.acceleration.y * event.acceleration.y 
                         + event.acceleration.z * event.acceleration.z);

  module.bus.send(attitude);

  Packet imu(buf, sizeof(buf));
  imu.set(TELEMETRY, 'i');

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu.end()
    .append('g', (float)event.gyro.x)
    .append('g', (float)event.gyro.y)
    .append('g', (float)event.gyro.z);

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  imu.end()
    .append('m', (float)event.magnetic.x)
    .append('m', (float)event.magnetic.y)
    .append('m', (float)event.magnetic.z);

  module.bus.send(imu);

  if (accel > 35) sequence = FlightSequence::BURNING;
  else if (sequence == FlightSequence::BURNING) {
    sequence = FlightSequence::ASCENDING;
  }
  // Serial.println(accel);

  // module.indicator.blink(1);
}

void send_altitude() {
  
  float pressure_altitude_next = bme.readAltitude(QNH_HPA);
  float ascent_rate_raw = (pressure_altitude_next - pressure_altitude) * SEND_ALT_FREQ;
  ascent_rate = ascent_rate_raw * (1 - ASCENT_RATE_FILTER_A) + ascent_rate * ASCENT_RATE_FILTER_A;
  pressure_altitude = pressure_altitude_next;

  if (sequence == FlightSequence::ASCENDING && ascent_rate < -1.0) 
    sequence = FlightSequence::DESCENDING;
  else if (sequence == FlightSequence::DESCENDING && -1.0 < ascent_rate && ascent_rate < 1.0 && pressure_altitude < 1000)
    sequence = FlightSequence::LANDED;

  uint8_t buf[BUF_SIZE(7)]; 
  Packet atmosphere(buf, sizeof(buf));
  atmosphere.set(TELEMETRY, 'a', BROADCAST);
  atmosphere.begin()
    .append('I', float16(bme.readTemperature()))
    .append('P', (float)bme.readPressure())
    .append('A', pressure_altitude)
    .append('H', float16(bme.readHumidity()))
    .append('R', float16(ascent_rate))
    .append('f', sequence);

  // Serial.printf("%f %f\n", pressure_altitude, ascent_rate);

  // atmosphere.print();

  module.bus.send(atmosphere);

  module.indicator.blink(10);

  // Serial.println('.');
}

void loop() {
  static FlightMode mode_prev = mode.value();

  // if (mode_prev != mode.value()) {
  //   if (mode.isValid() && mode.value() == FlightMode::FLIGHT) {
  //     task_send_attitude.setInterval(TASK_SECOND / SEND_ATT_FREQ);
  //     task_send_altitude.setInterval(TASK_SECOND / SEND_ALT_FREQ);
  //   } else {
  //     task_send_attitude.setInterval(TASK_SECOND / SEND_ATT_FREQ_STANDBY);
  //     task_send_altitude.setInterval(TASK_SECOND / SEND_ALT_FREQ_STANDBY);
  //   }
  // }
  mode_prev = mode.value();

  scheduler.execute();
  module.update();
}

void loop1() {
  
}
