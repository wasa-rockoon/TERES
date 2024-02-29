#include <TaskScheduler.h>
#include <SoftwareSerial.h>
#include <map>
#include <ctype.h>

#include "Shared.hpp"
#include "Packet.hpp"
#include "RP2040Module.h"
#include "E220.hpp"
#include "Datatypes.h"

/* #define DEBUG */

#define UNIT 0x1
#define NODE 'C'

#define LORA_ADDR 0xFFFF
#define LORA_CHANNEL 17

// Constants
#define SEND_RECOVERY_FREQ 0.1

#define TWE_BAUD 115200
#define TWE_APP_ID "57415341"
#define TWE_DEV_ID 0
#define TWE_CHANNEL "12,18,25"
#define TWE_DEST_ID 0x78

#define TWE_MAX_FREQ_PER_MESSAGE 11

#define LORA_PACKET_SIZE_MAX 64


// Tasks

Scheduler scheduler;

void send_recovery();
Task task_send_recovery(
  TASK_SECOND / SEND_RECOVERY_FREQ, TASK_FOREVER,
  &send_recovery, &scheduler, false);


// Objects

RP2040Module module(NODE);

SoftwareSerial lora_serial(LORA_RX_PIN, LORA_TX_PIN);
uint8_t lora_buf[256+64];
E220 lora(lora_serial, lora_buf, 256, 64);

// SoftwareSerial twe_serial(TWE_RX_PIN, TWE_TX_PIN);
// TWELITE twelite(twe_serial);
// unsigned twelite_error_count;

// Variables
std::map<uint8_t, unsigned long> twe_last_sent_millis;

bool lora_ok;
unsigned lora_busy_count = 0;

Shared<FlightMode> flight_mode(FlightMode::STANDBY);

uint8_t recovery_seq = 0;
Shared<int16_t> battery_V;
Shared<int32_t> longitude;
Shared<int32_t> latitude;
Shared<float>   gps_altitude;
Shared<float>   pressure_altitude;
Shared<float16> ascent_rate;
Shared<FlightSequence> sequence;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  bool module_ok = module.begin();

  lora_ok = lora.begin();
  lora_ok &= setupLora();

  // twe_serial.begin(TWE_BAUD);
  // twelite.listen(twelite_receive);
  // twelite.setDestination(TWE_DEST_ID);
  // bool twe_ok = twelite.begin();

  module.bus.subscribe(battery_V, TELEMETRY, 'B', 'B').setTimeout(1000);
  module.bus.subscribe(longitude, TELEMETRY, 'P', 'O').setTimeout(5000);
  module.bus.subscribe(latitude, TELEMETRY, 'P', 'A').setTimeout(5000);
  module.bus.subscribe(gps_altitude, TELEMETRY, 'P', 'H').setTimeout(5000);
  module.bus.subscribe(pressure_altitude, TELEMETRY, 'a', 'A').setTimeout(1000);
  module.bus.subscribe(ascent_rate, TELEMETRY, 'a', 'R').setTimeout(1000);
  module.bus.subscribe(sequence, TELEMETRY, 'a', 'f').setTimeout(2000);
  module.bus.subscribe(flight_mode, TELEMETRY, 'B', 'm').setTimeout(2000);

  // delay(1000);
  // twe_config();

  module.bus.sanity(1, lora_ok);

  task_send_recovery.enable();

  if (module_ok && lora_ok)
    module.indicator.clearError();
}

bool initialized = false;

bool setupLora() {
  bool ok = true;
  ok &= lora.setMode(E220::Mode::CONFIG_DS);
  ok &= lora.setParametersToDefault();
  ok &= lora.setSerialBaudRate(9600);
  ok &= lora.setDataRate(E220::SF::SF9, E220::BW::BW125kHz);
  ok &= lora.setEnvRSSIEnable(true);
  ok &= lora.setSendMode(E220::SendMode::TRANSPARENT);
  ok &= lora.setModuleAddr(LORA_ADDR);
  ok &= lora.setChannel(LORA_CHANNEL);
  ok &= lora.setRSSIEnable(false);
  ok &= lora.setMode(E220::Mode::NORMAL);
  return ok;
}

void send_recovery() {

  // Message message('C', 0, 3);
  // message.entries[0].set('Q', (int32_t)twelite.getLQI());
  // message.entries[1].set('S', (uint32_t)twelite.getSentCount());
  // message.entries[2].set('R', (uint32_t)twelite.getReceivedCount());

  if (lora.isBusy() || !lora_ok) {
      module.bus.error("LBS");
      Serial.println("OV");
      lora_busy_count++;
      if (lora_busy_count > 10) {
        lora.setMode(E220::Mode::CONFIG_DS);
        delay(100);
        lora.setMode(E220::Mode::NORMAL);
      }
      return;
  }
  lora_busy_count = 0;

  Serial.println("send");

  static unsigned packet_round = 0;

  switch (packet_round) {
  case 0: {
    Rocket rocket;
    rocket.mode = flight_mode.value();
    rocket.sequence = sequence.value();

    uint8_t buf[LORA_PACKET_SIZE_MAX];
    Packet recovery(buf, sizeof(buf));
    recovery.set(TELEMETRY, 'r', UNIT, BROADCAST);
    recovery.from(UNIT);
    recovery.setSeq(recovery_seq++);
    battery_V.appendIfValid(recovery, 'B');
    latitude.appendIfValid(recovery, 'A');
    longitude.appendIfValid(recovery, 'O');
    gps_altitude.appendIfValid(recovery, 'H');
    pressure_altitude.appendIfValid(recovery, 'h');
    ascent_rate.appendIfValid(recovery, 'R');
    recovery.end().append('r', rocket);
    recovery.writeCRC();

    lora.sendTransparent(recovery.buf, recovery.len);
    recovery.print();
    module.bus.send(recovery);

    packet_round++;
    break;
  }
  case 1: {
    Packet& sanity = module.bus.getSanitySummary();
    sanity.set(TELEMETRY, sanity.id(), UNIT, BROADCAST);
    sanity.from(UNIT);
    sanity.setSeq(recovery_seq++);
    sanity.writeCRC();

    lora.sendTransparent(sanity.buf, sanity.len);
    sanity.print();
    module.bus.send(sanity);

    packet_round++;
    break;
  }
  case 2: {
    Packet& errors = module.bus.getErrorSummary();
    errors.set(TELEMETRY, errors.id(), UNIT, BROADCAST);
    errors.from(UNIT);
    errors.setSeq(recovery_seq++);
    errors.writeCRC();
    errors.print();
    lora.sendTransparent(errors.buf, errors.len);
    errors.print();
    module.bus.send(errors);

    packet_round = 0; 
    break;
  }
  }

  // twelite_send(recovery);

  module.indicator.blink(200);
}


void loop() {
  // put your main code here, to run repeatedly:

  /* while (Serial.available() > 0) { */
  /*   twe_serial.printf("%c", Serial.read()); */
  /* } */

  // Serial.print('l');


  uint8_t* rx;
  unsigned len = lora.receive(rx);

  if (len > 0) {
    Packet launcher(rx, len, len);
    launcher.print();
  }

  scheduler.execute();
}

void loop1() {
  module.update();
}
