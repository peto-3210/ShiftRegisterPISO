#ifndef SHIFT_REGISTER_PISO_H
#define SHIFT_REGISTER_PISO_H

#include <Arduino.h>

class PISORegister {
    private:

    //Configuration variables
    int clkPin = 0;
    int ldPin = 0;
    int qhPin = 0;
    int pinNumber = 0;

    bool inputLogic = true;
    bool clkPolarity = true;
    bool ldPolarity = false;

    unsigned long ldClkPulseGapUs = 0;
    unsigned long pulseWidthUs = 100;       //half of the clock period -> 5000 Hz
    unsigned long readingDelay = 0;
    uint16_t validInputLoopNumber = 1;

    //Internal variables for pulse functions
    uint8_t edgeCount = 0;
    unsigned long lastEdgeTimestamp = 0;

    //Internal variables for reading function
    bool phase = false;                     //false for LD pulse, true for clk pulses
    int bitIndex = 0;                       //index of the next input bit to be stored
    int clkPulseCount = 0;                  //clock pulses issued during the current reading
    unsigned long lastReadingTimestamp = 0;
    uint16_t constantInputLoopsCounter = 0;

    //Data variables
    uint64_t validInputData = 0;
    uint64_t lastInputData = 0;
    uint64_t currentInputData = 0;

    /**
     * @brief Generates single pulse, f.e. rising and falling edge (or vice versa)
     * @param pin Pin for pulse
     * @param polarity True for rising edge first, false otherwise
     * @return False when pulsing, true when done
     */
    bool GeneratePulse(int pin, bool polarity);

    /**
     * @brief Generates "nested" rising and falling edge pulse (one pulse inside another).
     * The outer pulse will start 1 ldClkPulseGapUs before the start of inner pulse and will end
     * 1 ldClkPulseGapUs after the inner pulse ended. This is useful when register requires clock
     * pulse for asynchronous load.
     * @param innerPin Pin for inner pulse
     * @param innerPolarity True for rising edge first, false otherwise
     * @param outerPin Pin for outer pulse
     * @param outerPolarity True for rising edge first, false otherwise
     * @return False when pulsing, true when done
     */
    bool GenerateNestedPulse(int innerPin, bool innerPolarity, int outerPin, bool outerPolarity);

    /**
     * @brief Resets the internal state machine and data buffers.
     */
    void Reset(){
        edgeCount = 0;
        lastEdgeTimestamp = 0;
        phase = false;
        bitIndex = 0;
        clkPulseCount = 0;
        lastReadingTimestamp = 0;
        constantInputLoopsCounter = 0;
        validInputData = 0;
        lastInputData = 0;
        currentInputData = 0;
    }

    public:
    PISORegister(){}

    /**
     * @brief Prepares for reading
     * @param pinNumber Number of register inputs (1 to 64)
     * @param clkPin Pin used for clock signal
     * @param ldPin Pin used for asynchronous load signal
     * @param qhPin Input from shift register
     * @param clkPolarity Polarity of clock edge - true for rising edge, false for falling edge
     * @param ldPolarity Logic level of asynchronous loading - false for 0, true for 1
     * @param inputLogic Type of input logic (true for normal, false for inverse)
     */
    void Init(int pinNumber, int clkPin, int ldPin, int qhPin, bool clkPolarity, bool ldPolarity, bool inputLogic){
        if (pinNumber > 64){
            pinNumber = 64;
        }
        if (pinNumber < 1){
            pinNumber = 1;
        }
        this->pinNumber = pinNumber;

        this->clkPin = clkPin;
        this->clkPolarity = clkPolarity;
        this->ldPin = ldPin;
        this->ldPolarity = ldPolarity;
        this->qhPin = qhPin;
        this->inputLogic = inputLogic;

        Reset();

        pinMode(clkPin, OUTPUT);
        digitalWrite(clkPin, !clkPolarity);
        pinMode(ldPin, OUTPUT);
        digitalWrite(ldPin, !ldPolarity);
        pinMode(qhPin, INPUT);
    }

    /**
     * @brief Set delay between 2 readings (in microseconds, default is 0)
     */
    void SetReadingDelay(unsigned long readingDelay){
        this->readingDelay = readingDelay;
    }

    /**
     * @brief In order to mitigate the glitch occurence, the input must stay constant
     * during this many consecutive reading loops to be considered valid.
     * 1 (the default) means no filtering - every reading is accepted.
     */
    void SetGlitchPrevention(uint16_t validInputLoopNumber){
        if (validInputLoopNumber < 1){
            validInputLoopNumber = 1;
        }
        this->validInputLoopNumber = validInputLoopNumber;
    }

    /**
     * @brief Sets frequency of clock signal (in Hz, default value is 5000)
     * NOTE: Calling this function will reset ldClkPulseGapUs
     */
    void SetFrequency(unsigned long frequency){
        if (frequency == 0){
            frequency = 1;
        }
        //pulseWidthUs is half of the clock period
        unsigned long width = 500000UL / frequency;
        if (width < 1){
            width = 1;
        }
        this->pulseWidthUs = width;
        this->ldClkPulseGapUs = 0;
    }

    /**
     * @brief Used to generate single clock pulse during asynchronous loading,
     * between 2 edges of loading pulse. Some registers require this additional pulse
     * to load inputs. This is the delay (in microseconds) between the edge
     * of loading signal and clk signal, which must be less or equal to pulseWidth.
     * Set to 0 to disable.
     * NOTE: In this case, loading signal pulse will be wider than pulseWidth.
     */
    void SetLoadingAndClockPulseGap(unsigned long ldClkPulseGapUs){
        if (ldClkPulseGapUs > pulseWidthUs){
            ldClkPulseGapUs = pulseWidthUs;
        }
        this->ldClkPulseGapUs = ldClkPulseGapUs;
    }

    /**
     * @return Number of configured register inputs
     */
    int GetPinNumber() const { return pinNumber; }

    /**
     * @return Data from all inputs (raw, without inputLogic applied)
     */
    uint64_t GetAllInputData() const { return validInputData; }

    /**
     * @brief Generates loading pulse.
     * @return False when pulsing, true when done
     */
    bool GenerateLoadPulse();

    /**
     * @brief Shifts data and reads input. Generates pinNumber clock pulses per reading
     * @return True when all data was read, false otherwise
     */
    bool ShiftAndRead();

    /**
     * @brief Verifies and stores input data if valid.
     * @return True if data was stored, false otherwise
     */
    bool ValidateInput();

    /**
     * @brief Reads data from shift register.
     * Should be called in loop.
     */
    void ReadData();

    /**
     * @brief Reads data from desired input
     * @param num Input number (0 to pinNumber-1)
     * @returns Input data, false if num is out of range
     */
    bool GetInput(uint8_t num) const {
        if (num >= (uint8_t)pinNumber){
            return false;
        }
        return ((validInputData & ((uint64_t)1 << num)) != 0) == inputLogic;
    }

};

#endif
