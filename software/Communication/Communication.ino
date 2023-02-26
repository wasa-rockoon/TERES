#include <TaskScheduler.h>
#include <SoftwareSerial.h>

#include "RP2040Module.h"
#include "TWELITE.h"



// Constants
#define SEND_CONNECTION_FREQ 1


#define TWE_BAUD 115200
#define TWE_APP_ID "57415341"
#define TWE_DEV_ID 0
#define TWE_CHANNEL "12,18,25"
#define TWE_DEST_ID 0x78

// Tasks

Scheduler scheduler;

void send_connection();
Task task_send_connection(
  1000 / SEND_CONNECTION_FREQ * TASK_MILLISECOND, TASK_FOREVER,
  &send_connection, &scheduler, false);



// Objects

RP2040Module module;

SoftwareSerial twe_serial(TWE_RX_PIN, TWE_TX_PIN);
TWELITE twelite(twe_serial);
unsigned twelite_error_count;


void setup() {
  // put your setup code here, to run once:

  bool module_ok = module.begin();

  twe_serial.begin(TWE_BAUD);
  twelite.listen(twelite_receive);
  twelite.setDestination(TWE_DEST_ID);
  bool twe_ok = twelite.begin();

  // delay(1000);
  // twe_config();

  module.bus.listen(twelite_send);

  task_send_connection.enable();

  if (module_ok && twe_ok)
    module.indicator.clearError();
}

void send_connection() {

  Message message('C', 0, 3);
  message.entries[0].set('Q', (int32_t)twelite.getLQI());
  message.entries[1].set('S', (uint32_t)twelite.getSentCount());
  message.entries[2].set('R', (uint32_t)twelite.getReceivedCount());

  module.bus.send(message);

  twelite_send(message);
}

void twelite_send(const Message& message) {
  Message reduced = message;

  Serial.println("TWE TX");
  message.print();

  twelite.send(reduced);

  module.indicator.blink(1);
}

void twelite_receive(const Message& message) {
  Serial.println("TWE RX");
  message.print();
}


void loop() {
  // put your main code here, to run repeatedly:

  /* while (Serial.available() > 0) { */
  /*   twe_serial.printf("%c", Serial.read()); */
  /* } */

  if (twelite_error_count != twelite.getErrorCount()) {
    module.indicator.errorEvent();
    twelite_error_count = twelite.getErrorCount();
  }

  scheduler.execute();
}


void twe_config() {
  throgh();
  twe_serial.print("+");
  delay(500);
  twe_serial.print("+");
  delay(500);
  twe_serial.print("+");
  throgh(); delay(1); throgh();
  twe_serial.print("a");
  twe_serial.println(TWE_APP_ID);
  throgh(); delay(1); throgh();
  twe_serial.print("i");
  twe_serial.println(TWE_DEV_ID);
  throgh(); delay(1); throgh();
  twe_serial.print("c");
  twe_serial.println(TWE_CHANNEL);
  throgh(); delay(1); throgh();
  twe_serial.print("m");
  twe_serial.println("B");
  throgh(); delay(1); throgh();
  twe_serial.println("S");
  throgh(); delay(1); throgh();
}

void throgh() {
  while (twe_serial.available() > 0) {
    Serial.printf("%c", twe_serial.read());
  }
}