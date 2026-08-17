#include <Arduino.h>
#include <math.h>

#define LDR_PIN 4

constexpr double ADCmax = 4095.0;
constexpr double Uref = 3100.0;

void setup(){
  Serial.begin(115200);
  delay(1000);

  pinMode(LDR_PIN, INPUT);
}

void loop(){

  Serial.print("Start \n");

  double raw = analogRead(LDR_PIN);

  Serial.print("Raw value: ");
  Serial.println(raw);

  double U_calc = (raw/ADCmax) * Uref;

  Serial.print("U_calc calculated value: ");
  Serial.println(U_calc);

  double raw2 = analogReadMilliVolts(LDR_PIN);

  Serial.print("U_calc calculated value: ");
  Serial.println(raw2);

  double Fault_calc = (raw2 != 0) ? (fabs(U_calc - raw2) / raw2) * 100 : 0.0;

  Serial.print("Fault_calc calculated value: ");
  Serial.print(Fault_calc);
  Serial.println('%');

  Serial.print("End \n");
  delay(1000);
}