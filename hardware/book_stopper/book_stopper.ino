#include <ArduinoJson.h>

#include "Stopper.h"


void setup() {
  Serial.begin(115200);

  Stopper::getInstance().init();
}

void loop() {
  Stopper::getInstance().run();
}
