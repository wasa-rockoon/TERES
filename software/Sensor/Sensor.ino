
#include <TaskScheduler.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>


#include "RP2040Module.h"

#define LED_STATUS 0
#define LED_ERROR 25

#define BNO055_ADDR 0x28
#define BNE280_ADDR 0x76

#define SEND_ATT_FREQ 25
#define SEND_ATT_FREQ_STANDBY 1
#define SEND_ALT_FREQ 10
#define SEND_ALT_FREQ_STANDBY 1

#define QNH_HPA (1013.25)

// Tasks

Scheduler scheduler;

void send_attitude();
Task task_send_attitude(TASK_SECOND / SEND_ATT_FREQ_STANDBY,
                        TASK_FOREVER, &send_attitude, &scheduler, false);

void send_altitude();
Task task_send_altitude(TASK_SECOND / SEND_ALT_FREQ_STANDBY,
                        TASK_FOREVER, &send_altitude, &scheduler, false);


// Objects

RP2040Module module;

Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO055_ADDR, &Wire1);
Adafruit_BME280 bme;


void setup() {
  // put your setup code here, to run once:

  bool module_ok = module.begin();

  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire1.begin();

  bool bno_ok = bno.begin();

  bool bme_ok = bme.begin(BNE280_ADDR, &Wire1);

  delay(1000);
>>>>>>> 664bd85 (センサー完成)

  if (bno_ok) task_send_attitude.enable();
  if (bme_ok) task_send_altitude.enable();

  module.bus.listen('m', changeMode);

  if (module_ok && bno_ok && bme_ok) {
    module.indicator.clearError();
  }
}

void send_attitude() {

  Message tlm('A', 0, 13);

  sensors_event_t event;
  bno.getEvent(&event);

  imu::Quaternion q = bno.getQuat();
  tlm.entries[0].set('Q', (float)q.x());
  tlm.entries[1].set('Q', (float)q.y());
  tlm.entries[2].set('Q', (float)q.z());
  tlm.entries[3].set('Q', (float)q.w());

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  tlm.entries[4].set('A', (float)event.acceleration.x);
  tlm.entries[5].set('A', (float)event.acceleration.y);
  tlm.entries[6].set('A', (float)event.acceleration.z);

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);
  tlm.entries[7].set('g', (float)event.gyro.x);
  tlm.entries[8].set('g', (float)event.gyro.y);
  tlm.entries[9].set('g', (float)event.gyro.z);

  bno.getEvent(&event, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  tlm.entries[10].set('m', (float)event.magnetic.x);
  tlm.entries[11].set('m', (float)event.magnetic.y);
  tlm.entries[12].set('m', (float)event.magnetic.z);

  // tlm.print();

  module.bus.send(tlm);

  module.indicator.blink(1);
}


void send_altitude() {
  Message tlm('H', 0, 4);
  tlm.entries[0].set('T', (float)bme.readTemperature());
  tlm.entries[1].set('P', (float)bme.readPressure());
  tlm.entries[2].set('A', (float)bme.readAltitude(QNH_HPA));
  tlm.entries[3].set('h', (float)bme.readHumidity());

  // tlm.print();

  module.bus.send(tlm);

  module.indicator.blink(1);

}

void changeMode(const Message& message) {
  /* if (message.get('F')) { */
    task_send_attitude.setInterval(TASK_SECOND / SEND_ATT_FREQ);
    task_send_altitude.setInterval(TASK_SECOND / SEND_ALT_FREQ);
  /* } */
  /* else if (message.get('S')) { */
  /*   task_send_attitude.setInterval(TASK_SECOND / SEND_ATT_FREQ_STANDBY); */
  /*   task_send_altitude.setInterval(TASK_SECOND / SEND_ALT_FREQ_STANDBY); */
  /* } */
}

void loop() {
  // put your main code here, to run repeatedly:

  scheduler.execute();
}
