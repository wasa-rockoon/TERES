#include <TaskScheduler.h>

#define BUS_DEBUG_SERIAL

#include "RP2040Module.h"

Scheduler scheduler;

RP2040Module module('t');

void blink();

Task task_blink(1000 * TASK_MILLISECOND, TASK_FOREVER, &blink, &scheduler, true);

queue_t queue;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  delay(100);

  bool ok = module.begin();

  delay(1000);
  
  module.indicator.clearError();

  queue_init(&queue, 1, 1024);
}

void blink() {
  module.indicator.blink();

  uint8_t buf[BUF_SIZE(3)];
  Packet test(buf, sizeof(buf));
  test.set(TELEMETRY, 'T', 0x1, TO_LOCAL);
  test.begin()
    .append('I', (int32_t)123)
    .append('F', (float)56.7)
    .append('G', (float)89.1);

  module.bus.send(test);

// #ifndef BUS_DEBUG_SERIAL
  // Serial.printf("error: %d, drop: %d\n", module.bus.getErrorCount(), module.bus.getDroppedCount());
  // module.bus.printNodeInfo();
// #endif
}

void loop() {
  // put your main code here, to run repeatedly:
 
  scheduler.execute();
}

void loop1() {
  // delay(100);
  module.update();
  // rp2040.fifo.push(i++);
}

