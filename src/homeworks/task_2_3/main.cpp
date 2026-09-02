#include <Arduino.h>

#define LED_PIN1 2
#define LED_PIN2 17
#define LED_PIN3 18

constexpr uint16_t BLINK_INTERVAL_MS1 = 200;
constexpr uint16_t BLINK_INTERVAL_MS2 = 500;
constexpr uint16_t BLINK_INTERVAL_MS3 = 1000;

void setup()
{
  Serial.begin(115200);
  delay(100); // Wait for Serial to initialize
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  Serial.println("Demo 02: serial counter started");
}

void loop()
{
  static unsigned long prev1 = 0;
  static unsigned long prev2 = 0;
  static unsigned long prev3 = 0;

  unsigned long now = millis();

  if (now - prev1 >= BLINK_INTERVAL_MS1) {
    prev1 = now;
    digitalWrite(LED_PIN1, !digitalRead(LED_PIN1));
  }
  if (now - prev2 >= BLINK_INTERVAL_MS2) {
    prev2 = now;
    digitalWrite(LED_PIN2, !digitalRead(LED_PIN2));
  }
  if (now - prev3 >= BLINK_INTERVAL_MS3) {
    prev3 = now;
    digitalWrite(LED_PIN3, !digitalRead(LED_PIN3));
  }
}
