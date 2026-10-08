#include <Arduino.h>
#include <ShiftRegisterPISO.h>

#define clkPin 5
#define ldPin 4
#define qhPin 0
#define inputCount 8

PISORegister r1;

void setup() {
  Serial.begin(115200);
  r1.Init(inputCount, clkPin, ldPin, qhPin, true, false, true);
  r1.SetReadingDelay(1000);
  r1.SetFrequency(10000);
  r1.SetLoadingClockPulse(50);
  r1.SetGlitchPrevention(3);
}

void loop() {
  r1.ReadData();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();
    for (uint8_t i = 0; i < inputCount; i++) {
      Serial.print(r1.GetInput(i) ? '1' : '0');
    }
    Serial.println();
  }
}
