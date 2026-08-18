#include <Arduino.h>
#include <BluetoothSerial.h>

#define LDR_PIN     4
#define BUTTON_PIN  15
#define LED1_PIN    19
#define RELE_PIN    23

const int LDR_DARK_THRESHOLD  = 2500; // raw нижче -> темно -> реле ON
const int LDR_LIGHT_THRESHOLD = 2950; // raw вище  -> світло -> реле OFF

const unsigned long DEBOUNCE_MS   = 50;
const unsigned long LDR_PERIOD_MS = 100;
const bool RELAY_ACTIVE_LOW = false; 

BluetoothSerial SerialBT;

enum Mode { MODE_AUTO, MODE_MANUAL };

Mode mode = MODE_AUTO;
bool relayOn = false;                 

int lastButtonReading = HIGH;
int buttonState       = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long lastLdrTime = 0;

void applyRelay() {                    // єдина точка фізичного керування
  bool level = relayOn;
  if (RELAY_ACTIVE_LOW) level = !level;
  digitalWrite(RELE_PIN, level ? HIGH : LOW);
  digitalWrite(LED1_PIN, (mode == MODE_MANUAL) ? HIGH : LOW);
}

void onButtonPress() {                 // кнопка перемикає режим
  if (mode == MODE_AUTO) {
    mode = MODE_MANUAL;
    relayOn = true;                    // форсуємо світло
  } else {
    mode = MODE_AUTO;                  // LDR знову бере керування
  }
  Serial.printf("Button -> mode=%s relay=%d\n",
                mode == MODE_AUTO ? "AUTO" : "MANUAL", relayOn);
}

void handleButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) {
    lastDebounceTime = millis();       // рівень смикнувся -> перезапуск таймера
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != buttonState) {      // стабільний новий стан
      buttonState = reading;
      if (buttonState == LOW) {        // INPUT_PULLUP: натиснута = LOW
        onButtonPress();
      }
    }
  }
  lastButtonReading = reading;
}

void handleBluetooth() {               // BT дозволено ТІЛЬКИ вимикати
  if (!SerialBT.available()) return;
  char data = SerialBT.read();
  if (data == '0') {
    mode = MODE_MANUAL;
    relayOn = false;
    Serial.println("BT -> OFF (manual)");
  } else if (data == '1') {
    mode = MODE_MANUAL;
    relayOn = true;
    Serial.println("BT '1' ignored (off-only)");
  } else {
    Serial.println("BT invalid value");
  }
}

void handleLDR() {                     // працює лише в авто-режимі
  if (mode != MODE_AUTO) return;
  int raw = analogRead(LDR_PIN);
  if (raw < LDR_DARK_THRESHOLD) {
    relayOn = true;
  } else if (raw > LDR_LIGHT_THRESHOLD) {
    relayOn = false;
  }                                    // між порогами -> не міняємо
  Serial.printf("raw=%d relay=%d\n", raw, relayOn);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(RELE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  SerialBT.begin("ESP32_LED_LAMP");
  applyRelay();
  Serial.println("Init successful");
}

void loop() {
  handleButton();
  handleBluetooth();
  if (millis() - lastLdrTime >= LDR_PERIOD_MS) {
    lastLdrTime = millis();
    handleLDR();
  }
  applyRelay();                        // без delay()
}