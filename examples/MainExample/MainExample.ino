#include <Arduino.h>
#include <ShiftRegisterPISO.h>

#define clkPin 5
#define ldPin 4
#define qhPin 0
#define inputCount 8

PISORegister r1;

void setup() {
  r1.Init(clkPin, ldPin, qhPin, inputCount, true);
  r1.SetReadingDelay(1000);
  r1.SetLdClkPulseDelay(20);
  r1.SetPulseWidth(100);
}

void loop() {
  r1.ReadData();
}
