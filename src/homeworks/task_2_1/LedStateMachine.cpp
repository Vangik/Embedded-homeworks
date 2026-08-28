#include "LedStateMachine.h"  
#include "Config.h"


LedStateMachine::LedStateMachine(int ledPin, unsigned long blinkInterval)
{
    pin = ledPin;
    interval = blinkInterval;
    currentState = LedState::LED_OFF;
    lastToggle = 0;
    mode = LedMode::BLINK;
}

void LedStateMachine::init()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void LedStateMachine::update()
{
    switch (mode)
    {
        case LedMode::SOLID_ON:
            digitalWrite(pin, HIGH);
            digitalWrite(Config::LED_PIN2, HIGH);
            return;
        case LedMode::SOLID_OFF:
            digitalWrite(pin, LOW);
            digitalWrite(Config::LED_PIN2, HIGH);
            return;
        case LedMode::BLINK:
            digitalWrite(Config::LED_PIN2, LOW);
            break;
    }

    unsigned long currentMillis = millis();
    switch (currentState)
    {
        case LedState::LED_OFF:
            if (currentMillis - lastToggle >= interval)
            {
                digitalWrite(pin, HIGH);
                currentState = LedState::LED_ON;
                lastToggle = currentMillis;
            }
            break;

        case LedState::LED_ON:
            if (currentMillis - lastToggle >= interval)
            {
                digitalWrite(pin, LOW);
                currentState = LedState::LED_OFF;
                lastToggle = currentMillis;
            }
            break;
    }
}

void LedStateMachine::nextMode()
{
    switch (mode)
    {
        case LedMode::BLINK:     mode = LedMode::SOLID_ON;  break;
        case LedMode::SOLID_ON:  mode = LedMode::SOLID_OFF; break;
        case LedMode::SOLID_OFF: mode = LedMode::BLINK;     break;
    }
}
