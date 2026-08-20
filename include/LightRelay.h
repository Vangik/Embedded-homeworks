#ifndef LIGHT_RELAY_H
#define LIGHT_RELAY_H

#include <Arduino.h>
#include "Config.h"

extern bool relayState;

// LDR logic
float rawToLux(int raw);
float readLux();

bool isLuxBelowThreshold(float lux);
bool isLuxAboveThreshold(float lux);

bool shouldTurnOnRelay(float lux);
bool shouldTurnOffRelay(float lux);

// Relay logic
void turnOnRelay();
void turnOffRelay();

#endif // LIGHT_RELAY_H