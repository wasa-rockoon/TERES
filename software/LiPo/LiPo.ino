
#include <TaskScheduler.h>

#include "RP2040Module.h"
#include "Shared.hpp"
#include "Datatypes.h"


#define SEND_TLM_FREQ 10
#define SEND_TLM_RECOVERY_FREQ 1

#define FLIGHT_DURATION_MS 60000
#define BATTERY_V_MIN 3.5
#define SUPPLY_V_MIN 4.0
#define TEMP_MAX 80.0

#define ADC_SEL1_PIN 13
#define ADC_SEL2_PIN 20
#define ADC_SEL3_PIN 18

#define CHARGE_PIN 15
#define HEATER_PIN 14

#define POWER_VOLTAGE 3.7
#define THERMISTER_B 3380
#define THERMISTER_R 10000

// A0:(adcsel1 HIGH)SUPPLY_V, (adcsel2 HIGH)BAT_V
// A1:(adcsel1 HIGH)V+_I, (adcsel2 HIGH)HEATER_I
// A2:(adcsel1 HIGH)V+_V, (adcsel2 HIGH)TEMP, (adcsel3 HIGH)OB_TEMP


// Tasks

Scheduler scheduler;

void send_tlm();
Task task_send_tlm(TASK_SECOND / SEND_TLM_FREQ,
                   TASK_FOREVER, &send_tlm, &scheduler, false);


// Objects

RP2040Module module('B');

// Variables
FlightMode mode = FlightMode::STANDBY;

unsigned long flight_start_ms;

void setup() {
  // put your setup code here, to run once:

  bool module_ok = module.begin();

  pinMode(ADC_SEL1_PIN, OUTPUT);
  pinMode(ADC_SEL2_PIN, OUTPUT);
  pinMode(ADC_SEL3_PIN, OUTPUT);
  digitalWrite(ADC_SEL1_PIN, LOW);
  digitalWrite(ADC_SEL2_PIN, LOW);
  digitalWrite(ADC_SEL3_PIN, LOW);
  analogReadResolution(12);

  pinMode(CHARGE_PIN, INPUT_PULLUP);

  delay(100);

  flight_start_ms = millis();

  task_send_tlm.enable();

  if (module_ok) {
    module.indicator.clearError();
  }
}

void send_tlm() {

  digitalWrite(ADC_SEL1_PIN, HIGH);
  delay(1);
  float supply_V = analogRead(A0) / 4095.0 * 3.3 * 2.0;
  // float power_current = analogRead(A1) / 4095.0 * 3.3;
  // float power_V = analogRead(A2) / 4095.0 * 3.3 / 0.5 * POWER_VOLTAGE;

  digitalWrite(ADC_SEL1_PIN, LOW);
  delay(1);
  digitalWrite(ADC_SEL2_PIN, HIGH);
  delay(1);
  float battery_V = analogRead(A0) / 4095.0 * 3.3 * 2.0;
  // float heater_current = analogRead(A1) / 4095.0 * 3.3;
  // float env_temp = analogRead(A2) / 4095.0 * 3.3 / 0.5 * POWER_VOLTAGE;

  digitalWrite(ADC_SEL2_PIN, LOW);
  delay(1);
  digitalWrite(ADC_SEL3_PIN, HIGH);
  delay(1);
  float thermister_read = analogRead(A2);
  float thermister_resistance = THERMISTER_R * thermister_read / (4095.001 - thermister_read);
  float board_temp =
        1.0 / (1.0 / THERMISTER_B * log(thermister_resistance / THERMISTER_R) + 1.0/298.15) - 273.15;

  digitalWrite(ADC_SEL3_PIN, LOW);

  float ic_temp = analogReadTemp();


  bool charging = !digitalRead(CHARGE_PIN);

  bool supplying = supply_V > SUPPLY_V_MIN;
  bool battery_ok = battery_V > BATTERY_V_MIN;
  bool temp_ok = board_temp < TEMP_MAX;

  if (supplying) {
    mode = FlightMode::STANDBY;
    flight_start_ms = millis();
    task_send_tlm.setInterval(TASK_SECOND / SEND_TLM_FREQ);
    module.indicator.blink(5);
  }
  else if (millis() - flight_start_ms > FLIGHT_DURATION_MS || !battery_ok || !temp_ok) {
    mode = FlightMode::RECOVERY;
    task_send_tlm.setInterval(TASK_SECOND / SEND_TLM_RECOVERY_FREQ);
    module.indicator.blink(5);
  }
  else {
    mode = FlightMode::FLIGHT;
    task_send_tlm.setInterval(TASK_SECOND / SEND_TLM_FREQ);
    module.indicator.blink(50);
  }


  uint8_t buf[BUF_SIZE(6)]; 
  Packet battery(buf, sizeof(buf));
  battery.set(TELEMETRY, 'B');
  battery.end()
    .append('B', (int16_t)(battery_V * 1000.0))
    .append('S', (int16_t)(supply_V * 1000.0))
    // .append('P', (int16_t)(power_V * 1000.0))
    .append('T', (int16_t)board_temp)
    .append('t', (int16_t)ic_temp)
    .append('m', mode)
    .append('C', charging);

  // battery.print();

  module.bus.sanity(1, battery_ok);
  module.bus.sanity(2, temp_ok);

  module.bus.send(battery);
}


void loop() {

  scheduler.execute();
  module.update();
}

void loop1() {
  
}
