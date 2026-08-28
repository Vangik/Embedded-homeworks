#pragma once
#include <Arduino.h>

// Logging
#define DEBUG 1

class Config {
public:
    // Піни
    static constexpr uint8_t LED_PIN1   = 4;
    static constexpr uint8_t LED_PIN2   = 19;
    static constexpr uint8_t BUTTON_PIN = 21;

    // Часові параметри (мс)
    static constexpr uint16_t BLINK_INTERVAL_MS = 500;
    static constexpr uint8_t  DEBOUNCE_MS       = 50;

    // Додаткові налаштування (нове у завданні 2)
    static constexpr uint8_t  SHORT_PRESS_BLINK_COUNT = 3;    
    static constexpr uint16_t LONG_PRESS_MS           = 1000; 
};