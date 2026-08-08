//Basic task
#include <Arduino.h>

#define LED1_PIN 4
#define LED2_PIN 18  

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED1_PIN, HIGH);
  delay(1000);

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);
  delay(1000);

  digitalWrite(LED2_PIN, LOW);
}


//Improved blinking pattern
#include <Arduino.h>

#define LED1_PIN 4
#define LED2_PIN 18  

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {

  for(int i=0; i<5; i++){
    digitalWrite(LED1_PIN, HIGH);
    delay(500);
    digitalWrite(LED1_PIN, LOW);
    delay(500);
  }

  for(int i=0; i<5; i++){
    digitalWrite(LED2_PIN, HIGH);
    delay(500);
    digitalWrite(LED2_PIN, LOW);
    delay(500);
  }
}


//3 Leds blinking pattern
#include <Arduino.h>

#define LED1_PIN 4
#define LED2_PIN 5
#define LED3_PIN 18  

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
}

void loop() {

  for(int i=0; i<5; i++){
    digitalWrite(LED1_PIN, HIGH);
    delay(500);
    digitalWrite(LED1_PIN, LOW);
    delay(500);
  }

  for(int i=0; i<5; i++){
    digitalWrite(LED2_PIN, HIGH);
    delay(500);
    digitalWrite(LED2_PIN, LOW);
    delay(500);
  }

  for(int i=0; i<5; i++){
    digitalWrite(LED3_PIN, HIGH);
    delay(500);
    digitalWrite(LED3_PIN, LOW);
    delay(500);
  }
}


// Increase blinking speed
#include <Arduino.h>

#define LED1_PIN 4
#define LED2_PIN 5
#define LED3_PIN 18  

unsigned long timer1 = 200;
unsigned long timer2 = 600;
unsigned long timer3 = 900;

unsigned long prevMillis1 = 0;
unsigned long prevMillis2 = 0;
unsigned long prevMillis3 = 0;


bool LED1_state = false;
bool LED2_state = false;
bool LED3_state = false;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
}

void loop() {
  unsigned long now = millis();

  if((now - prevMillis1) > timer1){
    digitalWrite(LED1_PIN, LED1_state);
    LED1_state = !LED1_state;
    digitalWrite(LED1_PIN, LED1_state);
    prevMillis1 = now; 
  }

  if((now - prevMillis2) > timer2){
    digitalWrite(LED2_PIN, LED2_state);
    LED2_state = !LED2_state;
    digitalWrite(LED2_PIN, LED2_state);
    prevMillis2 = now; 
  }

  if((now - prevMillis3) > timer3){
    digitalWrite(LED3_PIN, LED3_state);
    LED3_state = !LED3_state;
    digitalWrite(LED3_PIN, LED3_state);
    prevMillis3 = now; 
  }
}



