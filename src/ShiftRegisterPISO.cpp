#include <ShiftRegisterPISO.h>


bool PISORegister::GeneratePulse(int pin, bool polarity){
    unsigned long currentEdgeTimestamp = micros();
    //Timer overflow
    if (currentEdgeTimestamp < lastEdgeTimestamp){
        lastEdgeTimestamp = 0;
    }

    //Beginning of wave - first edge
    if (currentEdgeTimestamp - lastEdgeTimestamp > pulseWidth){
        if (edgeCount == 0){
            digitalWrite(pin, polarity);
            edgeCount++;
        }

        //Second edge
        else if(edgeCount == 1){
            digitalWrite(pin, !polarity);
            edgeCount++;
        }

        //End of wave
        else {
            edgeCount = 0;
            return true;
        }
        lastEdgeTimestamp = currentEdgeTimestamp;
    }
    return false;
}

bool PISORegister::GenerateNestedPulse(int innerPin, bool innerPolarity, int outerPin, bool outerPolarity){
    unsigned long currentEdgeTimestamp = micros();
    //Timer overflow
    if (currentEdgeTimestamp < lastEdgeTimestamp){
        lastEdgeTimestamp = 0;
    }

    //Beginning of wave - First outer edge
    if (edgeCount == 0 && currentEdgeTimestamp - lastEdgeTimestamp > pulseWidth){
        digitalWrite(outerPin, outerPolarity);
        edgeCount++;
        lastEdgeTimestamp = currentEdgeTimestamp;
    }

    //First inner edge
    else if (edgeCount == 1 && currentEdgeTimestamp - lastEdgeTimestamp > ldClkPulseDelay){
        digitalWrite(innerPin, innerPolarity);
        edgeCount++;
        lastEdgeTimestamp = currentEdgeTimestamp;
    }

    //Second inner edge
    else if(edgeCount == 2 && currentEdgeTimestamp - lastEdgeTimestamp > pulseWidth){
        digitalWrite(innerPin, !innerPolarity);
        edgeCount++;
        lastEdgeTimestamp = currentEdgeTimestamp;
    }

    //Second outer edge
    else if (edgeCount == 3 && currentEdgeTimestamp - lastEdgeTimestamp > ldClkPulseDelay){
        digitalWrite(outerPin, !outerPolarity);
        edgeCount++;
        lastEdgeTimestamp = currentEdgeTimestamp;
    }

    //End of wave
    else if (currentEdgeTimestamp - lastEdgeTimestamp > pulseWidth){
        edgeCount = 0;
        return true;
    }
    return false;
}

void PISORegister::ReadData(){
    unsigned long currentReadingTimestamp = micros();
    //Timer overflow
    if (currentReadingTimestamp < lastReadingTimestamp){
        lastReadingTimestamp = 0;
    }
    if (currentReadingTimestamp - lastReadingTimestamp > readingDelay){

        //Loading phase, lasts for 1 pulse
        if (phase == false){

            //During loading pulse
            if (pulseCount == 0) {
                if (ldClkPulseDelay != 0){
                    pulseCount = GenerateNestedPulse(clkPin, clkPol, ldPin, ldPol);
                }
                else {
                    pulseCount = GeneratePulse(ldPin, ldPol);
                }
            }

            //After loading pulse
            else {
                pulseCount = 0;
                phase = true;

                //First input bit loads with LD pulse
                rawData |= (digitalRead(qhPin) << pulseCount++);
            }
        }

        //Reading phase, lasts for pinNum pulses
        else {
            if (GeneratePulse(clkPin, clkPol) == true){
                edgeCount = 0;
                //During reading
                if (pulseCount < pinNum){
                    rawData |= (digitalRead(qhPin) << pulseCount);
                    pulseCount++;
                }
                
                //Reading finished
                else {
                    //To remove possible glitches
                    if (tempData != rawData){
                        constantInputLoopsCounter = 1;
                        tempData = rawData;
                    }
                    else if (constantInputLoopsCounter < validInputLoopNumber){
                        constantInputLoopsCounter++;
                    }
                    else {
                        inputData = tempData;
                        constantInputLoopsCounter = 0;
                    }

                    rawData = 0;
                    pulseCount = 0;
                    phase = false;
                    lastReadingTimestamp = currentReadingTimestamp;
                }
            }
        }
    }
}