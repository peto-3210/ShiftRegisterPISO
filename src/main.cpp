#include <Arduino.h>
#include <PISORegister.h>

#define clkPin 5
#define ldPin 4
#define qhPin 0
#define inputCount 8

PISORegister r1;

void setup() {
  r1.Init(clkPin, ldPin, qhPin, inputCount, true);
  r1.SetReadingDelay(100);
  r1.SetLdClkPulseDelay(1);
  r1.SetPulseWidth(50);
}

void loop() {
  r1.ReadData();
}
