//Main task, blink LED1 and LED2 when button is pressed, blink LED1 and LED2 with a longer delay when boot button is pressed

#include <Arduino.h>

#define LED1_PIN 18
#define LED2_PIN 21
#define BUTTON_PIN 15
#define BOOT_BUTTON_PIN 0

bool buttonPressed = false;
bool bootButtonPressed = false;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  delay(1000); // Wait for serial monitor to open

  // Set LED pins as output
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Set button pin as input with pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Set boot button pin as input with pull-up resistor
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  int bootButtonState = digitalRead(BOOT_BUTTON_PIN); // Read the state of the boot button
  int buttonState = digitalRead(BUTTON_PIN); // Read the state of the boot button

  if (bootButtonState == LOW){
      digitalWrite(LED1_PIN, HIGH);
      delay(500);
      digitalWrite(LED2_PIN, HIGH);
      digitalWrite(LED1_PIN, LOW);
      delay(500);
      digitalWrite(LED2_PIN, LOW);
    }

  if (buttonState == LOW){
    digitalWrite(LED1_PIN, HIGH);
    delay(100);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED1_PIN, LOW);
    delay(100);
    digitalWrite(LED2_PIN, LOW);
  }

  delay(100); // Small delay to debounce button press
}

// Additional task
#include <Arduino.h>

#define LED1_PIN 18
#define LED2_PIN 21
#define BUTTON_PIN 15
#define BOOT_BUTTON_PIN 0


const int debounceDelay = 50;    // the debounce time; increase if the output flick
const int longPressDelay = 1000; // the delay for a long press

int lastDebounceTime = 0;  // the last time the output pin was toggled

int lastBootButtonState = LOW;
int lastButtonState = LOW;
int bootButtonState = LOW;
int buttonState = LOW;

void quickBlink();
void slowBlink();
void customBlink();

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  delay(1000); // Wait for serial monitor to open

  // Set LED pins as output
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Set button pin as input with pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Set boot button pin as input with pull-up resistor
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  pinMode(BUILTIN_LED, OUTPUT); // Set the built-in LED pin as output
  digitalWrite(BUILTIN_LED, HIGH); // Turn off the built-in LED
}

void loop()
{
  int bootButtonPressed = digitalRead(BOOT_BUTTON_PIN);
  int buttonPressed = digitalRead(BUTTON_PIN);

  if (bootButtonPressed != lastBootButtonState || buttonPressed != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (buttonPressed != buttonState && bootButtonPressed != bootButtonState &&
        buttonPressed == LOW && bootButtonPressed == LOW)
    {
      Serial.println("Both buttons pressed, executing custom blink pattern.");
      buttonState = buttonPressed;
      bootButtonState = bootButtonPressed;
      customBlink();
    }
    else if (bootButtonPressed != bootButtonState)
    {
      bootButtonState = bootButtonPressed;
      if (bootButtonState == LOW)
      {
        Serial.println("Boot button pressed, executing slow blink pattern.");
        slowBlink();
      }
    }
    else if (buttonPressed != buttonState)
    {
      buttonState = buttonPressed;
      if (buttonState == LOW)
      {
        Serial.println("External button pressed, executing quick blink pattern.");
        quickBlink();
      }
    }
  }

  lastBootButtonState = bootButtonPressed;
  lastButtonState = buttonPressed;
}

void quickBlink()
{
  for(;;){
    digitalWrite(LED1_PIN, HIGH);
    delay(200);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED1_PIN, LOW);
    delay(200);
    digitalWrite(LED2_PIN, LOW);

    if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(BOOT_BUTTON_PIN) == HIGH) {
      break; // Exit the loop if both buttons are released
    }
  };
}

void slowBlink()
{
  for(;;){
    digitalWrite(LED1_PIN, HIGH);
    delay(500);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED1_PIN, LOW);
    delay(500);
    digitalWrite(LED2_PIN, LOW);

    if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(BOOT_BUTTON_PIN) == HIGH) {
      break; // Exit the loop if both buttons are released
    }
  };
}

void customBlink()
{
  // rhythm pattern (ms) - creates a pulsing "light sound" feel
  const int pattern[] = {60, 60, 60, 200, 400};
  const int patternLen = sizeof(pattern) / sizeof(pattern[0]);

  for(;;){
    for (int i = 0; i < patternLen; i++) {
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, LOW);
      delay(pattern[i]);

      digitalWrite(LED1_PIN, LOW);
      digitalWrite(LED2_PIN, HIGH);
      delay(pattern[i]);

      if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(BOOT_BUTTON_PIN) == HIGH) {
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
        return; // Exit immediately if both buttons are released
      }
    }
  }
}

