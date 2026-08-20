#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Config.h"      
#include "Logger.h"  
#include "LightRelay.h"

enum Mode { MODE_AUTO, MODE_MANUAL };
Mode mode = MODE_AUTO;

int lastButtonReading = HIGH;   
int buttonState       = HIGH;
unsigned long lastDebounceTime = 0;

unsigned long lastLuxTime = 0;

BluetoothSerial SerialBT;

void updateModeLed() {
  digitalWrite(LED1_PIN, (mode == MODE_MANUAL) ? HIGH : LOW);
}

void enterManual(bool relayShouldBeOn) {
  mode = MODE_MANUAL;
  if (relayShouldBeOn) {
    turnOnRelay();
  } else {
    turnOffRelay();
  }
  updateModeLed();
}

void enterAuto() {
  mode = MODE_AUTO;
  updateModeLed();
}

void onButtonPress() {
  if (mode == MODE_AUTO) {
    enterManual(true);                
    log("Button -> MANUAL (relay ON)");
  } else {
    enterAuto();
    log("Button -> AUTO");
  }
}

void handleButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();       
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != buttonState) {      
      buttonState = reading;
      if (buttonState == LOW) {        
        onButtonPress();
      }
    }
  }

  lastButtonReading = reading;
}

void handleBluetooth() {
  if (!SerialBT.available()) return;

  char data = SerialBT.read();
  if (data == '1') {
    enterManual(true);
    log("BT -> ON (manual)");
  } else if (data == '0') {
    enterManual(false);
    log("BT -> OFF (manual)");
  } else {
    log("BT invalid value");
  }
}


void handleAutoMode() {
  if (mode != MODE_AUTO) return;      

  float lux = readLux();
  log("lux=", lux);                   

  if (shouldTurnOnRelay(lux)) {
    turnOnRelay();
    log("Auto -> relay ON");
  } else if (shouldTurnOffRelay(lux)) {
    turnOffRelay();
    log("Auto -> relay OFF");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(ADC_RESOLUTION_BITS);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  turnOffRelay();     
  updateModeLed();

  SerialBT.begin("ESP32_LED_LAMP");
  log("Init successful");
}

void loop() {
  handleButton();      
  handleBluetooth();

  if (millis() - lastLuxTime >= LUX_INTERVAL) { 
    lastLuxTime = millis();
    handleAutoMode();
  }
}