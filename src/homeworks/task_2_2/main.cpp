// Demo 02 — Serial counter
// Run with:  pio run -e 02_serial_counter -t upload
// Monitor:   pio device monitor -b 115200
//
// A second, totally independent demo. It also has its own setup()/loop().
// This one has NOTHING to do with demo 01 — that is the whole point:
// each [env:...] compiles exactly one of these files.

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Demo 02: serial counter started");
}

void loop() {
  static uint32_t n = 0;
  Serial.printf("count = %lu\n", (unsigned long)n++);
  delay(1000);
}
