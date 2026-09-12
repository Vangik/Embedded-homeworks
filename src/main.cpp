#include <Arduino.h>

#define RELAY_PIN 23           // керує обмоткою реле
#define RELAY_CONTACT_PIN 18   // читає сухий контакт (NO), INPUT_PULLUP

#define MEASUREMENTS 10        // скільки циклів міряємо
#define SETTLE_MS 500          // пауза, щоб реле й контакт заспокоїлись
#define TIMEOUT_US 500000UL    // 0.5 с — захист від "вічного очікування"

volatile unsigned long edgeTime = 0;    // коли спрацював контакт
volatile bool edgeCaptured = false;     // прапорець "перший фронт спіймано"


void runMeasurements();
void measureSwitch();

void IRAM_ATTR onContactChange() {
  if (!edgeCaptured) {
    edgeTime = micros();
    edgeCaptured = true;
  }
}


void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_CONTACT_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, LOW); 

  // CHANGE: ловимо і замикання (HIGH->LOW), і розмикання (LOW->HIGH)
  attachInterrupt(digitalPinToInterrupt(RELAY_CONTACT_PIN),
                  onContactChange, CHANGE);

  Serial.println("APP START");
  delay(500);

  runMeasurements();
}

void loop() {
  // все робиться один раз у setup через runMeasurements()
}

unsigned long measureSwitch(bool relayLevel) {
  edgeCaptured = false;               // "зводимо" прапорець ПЕРЕД командою
  unsigned long tStart = micros();    // засікаємо старт
  digitalWrite(RELAY_PIN, relayLevel);// подаємо команду на обмотку

  // чекаємо, поки ISR спіймає фронт (або поки не вийде таймаут)
  while (!edgeCaptured) {
    if (micros() - tStart > TIMEOUT_US) {
      return 0;                       // контакт не спрацював
    }
  }
  return edgeTime - tStart;           // затримка спрацювання
}

void runMeasurements() {
  unsigned long onSum = 0, offSum = 0;
  int onOk = 0, offOk = 0;

  Serial.println("N\tON (us)\tOFF (us)");

  for (int i = 1; i <= MEASUREMENTS; i++) {
    unsigned long onTime = measureSwitch(HIGH);   // час увімкнення
    delay(SETTLE_MS);                             // дати контакту заспокоїтись

    unsigned long offTime = measureSwitch(LOW);   // час вимкнення
    delay(SETTLE_MS);

    Serial.print(i);       Serial.print("\t");
    Serial.print(onTime);  Serial.print("\t");
    Serial.println(offTime);

    if (onTime  > 0) { onSum  += onTime;  onOk++;  }
    if (offTime > 0) { offSum += offTime; offOk++; }
  }

  Serial.println("---------------------");
  if (onOk)  { Serial.print("Avg ON:  "); Serial.print(onSum / onOk);   Serial.println(" us"); }
  if (offOk) { Serial.print("Avg OFF: "); Serial.print(offSum / offOk); Serial.println(" us"); }
  Serial.println("DONE");
}