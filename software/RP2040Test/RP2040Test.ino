#include <TaskScheduler.h>

#include "RP2040Module.h"

Scheduler scheduler;

RP2040Module module;

void blink();

Task task_blink(1000 * TASK_MILLISECOND, TASK_FOREVER, &blink, &scheduler, true);

void setup() {
  // put your setup code here, to run once:

  bool ok = module.begin();

  delay(1000);
  
  module.indicator.clearError();
}

void blink() {
  module.indicator.blink();

  Message test('T', 0, 2);
  test.entries[0].set('I', (int32_t)123);
  test.entries[1].set('F', (float)56.7);

  module.bus.send(test);

  Serial.print("ok: ");
  Serial.print(module.bus.getMessageCount());
  Serial.print(", error: ");
  Serial.println(module.bus.getErrorCount());
}

void loop() {
  // put your main code here, to run repeatedly:
 
  scheduler.execute();
}

