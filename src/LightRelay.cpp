#include "LightRelay.h"

bool relayState = false;

float rawToLux(int raw) {
  if (raw < 1) {
    raw = 1;
  }

  float voltage = (float)raw / ADC_MAX * VCC;
  float rLdr = R1 * (VCC - voltage) / voltage;

  return 10.0f * powf(RL10 / rLdr, 1.0f / GAMMA);
}

float readLux() {
  int raw = analogRead(LDR_PIN);
  return rawToLux(raw);
}

bool isLuxBelowThreshold(float lux) {
  return lux < LUX_ON;
}

bool isLuxAboveThreshold(float lux) {
  return lux > LUX_OFF;
}

bool shouldTurnOnRelay(float lux) {
  return isLuxAboveThreshold(lux) && !relayState;
}

bool shouldTurnOffRelay(float lux) {
  return isLuxBelowThreshold(lux) && relayState;
}

void turnOnRelay() {
  digitalWrite(RELAY_PIN, HIGH);
  relayState = true;
}

void turnOffRelay() {
  digitalWrite(RELAY_PIN, LOW);
  relayState = false;
}