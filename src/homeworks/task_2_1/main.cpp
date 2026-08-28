#include <Arduino.h>
#include "Config.h"
#include "LedStateMachine.h"
#include "Logger.h"

LedStateMachine blinker(Config::LED_PIN1, Config::BLINK_INTERVAL_MS);

static unsigned long iterationCounter = 0;

constexpr uint8_t debounceDelay = 50;
bool lastButtonState = LOW;
unsigned long lastDebounceTime = 0;

volatile bool buttonPressed = false;

void IRAM_ATTR button_isr() {
  buttonPressed = true;
  digitalWrite(Config::LED_PIN2, HIGH);  // червоний діод — індикація переривання (digitalWrite в ISR безпечний)
}

void setup() {
  Logger::init();
  blinker.init();
  pinMode(Config::BUTTON_PIN, INPUT_PULLUP);  
  pinMode(Config::LED_PIN2, OUTPUT);  

  attachInterrupt(digitalPinToInterrupt(Config::BUTTON_PIN), button_isr, FALLING);
  Logger::log("Setup complete");
}


void loop() {
  unsigned long start = micros();
  static unsigned long lastPress = 0;

  //task 4
  if (buttonPressed) {                 
    buttonPressed = false;             
    unsigned long now = millis();
    if (now - lastPress >= Config::DEBOUNCE_MS) {   
      blinker.nextMode();              
      lastPress = now;
      Logger::log("Mode changed");  
    }
  }


  //task 1 and 2
  blinker.update();


  //task 3
  unsigned long diff = micros() - start;   
  iterationCounter++;
  if (iterationCounter % 1000 == 0) {
    Logger::log("iteration time (us):", diff);
  }
}
