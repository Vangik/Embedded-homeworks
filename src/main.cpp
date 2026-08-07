//Basic task

#include <Arduino.h>

#define LED1_PIN 4
#define LED2_PIN 18  

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED1_PIN, HIGH);
  delay(1000);

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);
  delay(1000);

  digitalWrite(LED2_PIN, LOW);
}
