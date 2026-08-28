#include <Arduino.h>
#include "Logger.h"

// Define the functions inside the same namespace.

namespace Logger
{

  void init()
  {
#if DEBUG
    Serial.begin(115200);
    delay(100); // Wait for Serial to initialize
    Serial.println("Logger initialized");
#endif
  }

  void log(const char *message)
  {
#if DEBUG
    Serial.println(message);
#endif
  }

  void log(const char *message, float value)
  {
#if DEBUG
    Serial.print(message);
    Serial.println(value);
#endif
  }

} // namespace Logger
