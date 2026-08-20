#ifndef CONFIG_H
#define CONFIG_H

// Logging
#define DEBUG 1

// Pins
#define LDR_PIN 4
#define BUTTON_PIN 15
#define LED1_PIN 19
#define RELAY_PIN 23


// LDR divider / lux calibration (GL5528)
#define ADC_RESOLUTION_BITS 12
#define ADC_MAX ((float)((1 << ADC_RESOLUTION_BITS) - 1)) // 2^12 - 1 + float casting

#define VCC 3.3f
#define R1 10000.0f
#define GAMMA 0.7f
#define RL10 50000.0f // LDR resistance at 10 lux

// Timing
#define LUX_INTERVAL 100 // interval between lux readings in milliseconds

// Relay hysteresis
#define LUX_ON  450
#define LUX_OFF 500

// DEBOUNCE_MS 
#define DEBOUNCE_MS 50

#endif

