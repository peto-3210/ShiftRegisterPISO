#include <Arduino.h>
#include <ShiftRegisterPISO.h>

#define clkPin 5
#define ldPin 4
#define qhPin 0
#define inputCount 8

PISORegister r1;

void setup() {
  r1.Init(inputCount, clkPin, true, ldPin, false, qhPin, true);
  r1.SetReadingDelay(1000);
  r1.SetFrequency(10000);
  r1.SetLdClkPulseDelay(50);

}

void loop() {
  r1.ReadData();
}
