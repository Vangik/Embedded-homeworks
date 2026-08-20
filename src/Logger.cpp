#include <Arduino.h>
#include "Logger.h"

void log(const char* message) {
#if DEBUG
  Serial.println(message);
#endif
}

void log(const char* message, float value) {
#if DEBUG
  Serial.print(message);
  Serial.println(value);
#endif
}