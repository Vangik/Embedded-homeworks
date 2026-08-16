#include "Arduino.h"

#define BUTTON_PIN 15
bool lastState = HIGH;

void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  static int clickCount = 0;

  bool currentState = digitalRead(BUTTON_PIN); 

  if (currentState == LOW && lastState == HIGH) {
    clickCount++;
    Serial.print("Button clicked ");
    Serial.print(clickCount);
    Serial.println(" times.");
  }

  lastState = currentState;
}


#include <Arduino.h>


#define BUTTON_PIN 21
const int debounceDelay = 50; // The delay in milliseconds to debounce the button pin


int buttonState; // The button is pressed when the pin reads LOW
int lastButtonState = LOW; // The previous state of the button pin
unsigned long lastDebounceTime = 0; // The last time the button pin was read


void setup() {
 Serial.begin(115200); // Start the serial communication at 115200 baud rate
 pinMode(BUTTON_PIN, INPUT_PULLDOWN); // Set the button pin as an input with an internal pull-down resistor
 pinMode(2, OUTPUT); // Set the LED pin as an output
 Serial.println("Button state monitoring started");
}


void loop() {
 int reading = digitalRead(BUTTON_PIN); // Read the state of the button pin


 if (reading != lastButtonState) {
   // Reset the debounce timer
   lastDebounceTime = millis();
 }


 if((millis() - lastDebounceTime)> debounceDelay){
   if (reading != buttonState){


     buttonState = reading;


     if (buttonState == HIGH) {
       Serial.println("Button pressed");
       digitalWrite(2, HIGH); // Turn on the built-in LED when the button is pressed
     } else {
       Serial.println("Button released");
       digitalWrite(2, LOW); // Turn off the built-in LED when the button is released 
     }
   }
 }
 lastButtonState = reading; // Save the current state as the last state for the next loop
}
