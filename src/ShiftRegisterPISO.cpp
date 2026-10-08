#include <ShiftRegisterPISO.h>


bool PISORegister::GeneratePulse(int pin, bool polarity){
    unsigned long currentEdgeTimestamp = micros();
    if (currentEdgeTimestamp - lastEdgeTimestamp < pulseWidthUs){
        return false;
    }

    //Beginning of wave - first edge
    if (edgeCount == 0){
        digitalWrite(pin, polarity);
        edgeCount = 1;
        lastEdgeTimestamp = currentEdgeTimestamp;
        return false;
    }

    //Second edge - end of wave
    digitalWrite(pin, !polarity);
    edgeCount = 0;
    lastEdgeTimestamp = currentEdgeTimestamp;
    return true;
}

bool PISORegister::GenerateNestedPulse(int innerPin, bool innerPolarity, int outerPin, bool outerPolarity){
    unsigned long currentEdgeTimestamp = micros();

    //Time that has to elapse before the next edge of this waveform
    unsigned long requiredGap;
    switch (edgeCount){
        case 0:  requiredGap = pulseWidthUs;     break;  //idle time before the outer pulse
        case 1:  requiredGap = ldClkPulseGapUs;  break;  //outer edge -> inner edge
        case 2:  requiredGap = pulseWidthUs;     break;  //inner pulse width
        default: requiredGap = ldClkPulseGapUs;  break;  //inner edge -> outer edge
    }

    if (currentEdgeTimestamp - lastEdgeTimestamp < requiredGap){
        return false;
    }
    lastEdgeTimestamp = currentEdgeTimestamp;

    switch (edgeCount){
        //First outer (ld) edge
        case 0:
            digitalWrite(outerPin, outerPolarity);
            edgeCount = 1;
            return false;

        //First inner (clk) edge
        case 1:
            digitalWrite(innerPin, innerPolarity);
            edgeCount = 2;
            return false;

        //Second inner (clk) edge
        case 2:
            digitalWrite(innerPin, !innerPolarity);
            edgeCount = 3;
            return false;

        //Second outer (ld) edge - end of wave
        default:
            digitalWrite(outerPin, !outerPolarity);
            edgeCount = 0;
            return true;
    }
}

bool PISORegister::GenerateLoadPulse(){
    if (ldClkPulseGapUs != 0){
        return GenerateNestedPulse(clkPin, clkPolarity, ldPin, ldPolarity);
    }
    return GeneratePulse(ldPin, ldPolarity);
}

bool PISORegister::ShiftAndRead(){
    //pinNumber clock pulses are generated per reading
    if (clkPulseCount >= pinNumber){
        return true;
    }

    if (GeneratePulse(clkPin, clkPolarity) == true){
        clkPulseCount++;

        //Bit 0 is already read right after the loading pulse 
        //The last pulse only shifts the final bit out of the register
        if (bitIndex < pinNumber){
            currentInputData |= (uint64_t)(digitalRead(qhPin) != 0 ? 1 : 0) << bitIndex;
            bitIndex++;
        }
    }

    return clkPulseCount >= pinNumber;
}


bool PISORegister::ValidateInput(){

    //To remove possible glitches
    if (lastInputData != currentInputData){
        lastInputData = currentInputData;
        constantInputLoopsCounter = 1;
    }
    else if (constantInputLoopsCounter < 0xFFFF){
        constantInputLoopsCounter++;
    }

    if (constantInputLoopsCounter < validInputLoopNumber){
        return false;
    }

    validInputData = currentInputData;
    return true;
}

void PISORegister::ReadData(){
    unsigned long currentReadingTimestamp = micros();

    if (currentReadingTimestamp - lastReadingTimestamp < readingDelay){
        return;
    }

    //Loading phase, lasts for 1 pulse
    if (phase == false){
        if (GenerateLoadPulse() == true){
            //First input bit loads with LD pulse
            currentInputData = 0;
            bitIndex = 0;
            clkPulseCount = 0;
            currentInputData |= (uint64_t)(digitalRead(qhPin) != 0 ? 1 : 0) << bitIndex;
            bitIndex++;
            phase = true;
        }
    }

    //Reading phase, lasts for pinNumber pulses
    else {
        if (ShiftAndRead() == true){
            ValidateInput();

            bitIndex = 0;
            clkPulseCount = 0;
            phase = false;
            lastReadingTimestamp = micros();
        }
    }
}
