#include <TaskScheduler.h>
#include <SoftwareSerial.h>
#include <map>
#include <ctype.h>

#include "RP2040Module.h"
#include "TWELITE.h"

// #define DEBUG

// Constants
#define SEND_CONNECTION_FREQ 1


#define TWE_BAUD 115200
#define TWE_APP_ID "57415341"
#define TWE_DEV_ID 0
#define TWE_CHANNEL "12,18,25"
#define TWE_DEST_ID 0x78

#define TWE_MAX_FREQ_PER_MESSAGE 11

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


// Variables
std::map<uint8_t, unsigned long> twe_last_sent_millis;


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
  // Serial.println("SEND");
  // message.print();
  if (!isupper(message.id)) return;

  if (millis() - twe_last_sent_millis[message.id]
      < 1000 / TWE_MAX_FREQ_PER_MESSAGE) return;

  twe_last_sent_millis[message.id] = millis();


  Message reduced;
  reduced.id = message.id;
  reduced.from = message.from;
  reduced.size = 0;
  for (int n = 0; n < message.size; n++) {
    if (isupper(message.entries[n].type)) {
      reduced.entries[reduced.size] = message.entries[n];
      reduced.size++;
    }
  }

  if (reduced.size == 0) return;

#ifdef DEBUG
  reduced.print();
#endif

  twelite.send(reduced);

  module.indicator.blink(1);
}

void twelite_receive(const Message& message) {
#ifdef DEBUG
  Serial.println("TWE RX");
  message.print();
#endif

  module.bus.send(message);
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
