#ifndef LED_STATE_MACHINE_H
#define LED_STATE_MACHINE_H

#include <Arduino.h>

enum class LedState {
    LED_OFF,
    LED_ON
};

enum class LedMode {
    BLINK,
    SOLID_ON,
    SOLID_OFF
};


class LedStateMachine {
private:
    int pin;                    // GPIO pin number
    LedState currentState;      // Active FSM state
    unsigned long lastToggle;   // Timestamp of the last state change
    unsigned long interval; 
            // State duration in milliseconds
    LedMode mode;   // Current LED mode 

public:
  
    LedStateMachine(int ledPin, unsigned long blinkInterval);

    void init();
    void update();
    void nextMode();
};

#endif 
