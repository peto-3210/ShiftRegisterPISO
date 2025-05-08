#include <Arduino.h>

#ifndef SHIFT_STRUCT
#define SHIFT_STRUCT

class PISORegister {
    private:
    int clkPin = 0;
    int ldPin = 0;
    int qhPin = 0;
    int pinNum = 0;
    bool inputLogic = false;

    bool testVal = 0;


    bool clkPol = true;
    bool ldPol = false;
    unsigned long ldClkPulseDelay = 0;

    unsigned long pulseWidth = 100;
    unsigned long readingDelay = 0;
    int validInputLoopNumber = 2;

    //Internal variables for pulse functions
    int edgeCount = 0;
    int pulseCount = 0;

    bool phase = false; //false for LD pulse, true for clk pulses
    unsigned long lastEdgeTimestamp = 0;
    unsigned long lastReadingTimestamp = 0;

    /**
     * @brief Generates single pulse, f.e. rising and falling edge (or vice versa) 
     * @param pin Pin for pulse
     * @param polarity True for rising edge first, false otherwise
     * @return False when pulsing, true when done
     */
    bool GeneratePulse(int pin, bool polarity);

    /**
     * @brief Generates "nested" rising and falling edge pulse (one pulse inside another).
     * The outer pulse will start 1 ldClkPulseDelay before the start of inner pulse and will end 1 ldClkPulseDelay 
     * after the inner pulse ended. This is useful when register requires clock pulse for asynchronous load.
     * @param innerPin Pin for inner pulse
     * @param innerPolarity True for rising edge first, false otherwise
     * @param outerPin Pin for outer pulse
     * @param outerPolarity True for rising edge first, false otherwise
     * @return False when pulsing, true when done
     */
    bool GenerateNestedPulse(int innerPin, bool innerPolarity, int outerPin, bool outerPolarity);

    uint64_t inputData;
    
    uint64_t tempData;
    uint64_t rawData;

    uint16_t constantInputLoopsCounter;

    public:
    PISORegister(){}

    /**
     * @brief Prepares for reading
     * @param pinNum Number of register inputs (max 64)
     * @param clkPin Pin used for clock signal
     * @param clkPol Polarity of clock edge - true for rising edge, false for falling edge
     * @param ldPin Pin used for asynchronous load signal
     * @param ldPol Logic level of asynchronous loading - false for 0, true for 1
     * @param qhPin Output of shift register
     * @param inputLogic Type of input logic (true for normal, false for inverse)
     */
    void Init(int pinNum, int clkPin, bool cklPol, int ldPin, bool ldPol, int qhPin, bool inputLogic){
        this->clkPin = clkPin;
        this->clkPol = clkPol;
        this->ldPin = ldPin;
        this->ldPol = ldPol;
        this->qhPin = qhPin;
        this->pinNum = pinNum;
        if (pinNum > 64){
            pinNum = 64;
        }
        this->inputLogic = inputLogic;

        pinMode(clkPin, OUTPUT);
        digitalWrite(clkPin, !clkPol);
        pinMode(ldPin, OUTPUT);
        digitalWrite(ldPin, !ldPol);
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
     * during multiple reading loops to be considered valid (default is 1).
     */
    void SetGlitchPrevention(unsigned long validInputLoopNumber){
        this->validInputLoopNumber = validInputLoopNumber;
    }

    /**
     * @brief Sets frequency of clock signal (in Hz, default value is 5000)
     * NOTE: Calling this function will reset ldClkPulseDelay
     */
    void SetFrequency(unsigned long frequency){
        this->pulseWidth = (unsigned long)((2/(float)frequency) * 1000000);
        ldClkPulseDelay = 0;
    }

    /**
     * @brief If not zero, one clock pulse is generated during asynchronous loading,
     * between 2 edges of loading pulse. Some registers require this additional pulse 
     * to load inputs. This is the delay (in microseconds) between the edge
     * of loading signal and clk signal, which must be less or equal to pulseWidth. 
     * NOTE: In this case, loading signal pulse will be wider than pulseWidth.
     */
    void SetLdClkPulseDelay(unsigned long ldClkPulseDelay){
        this->ldClkPulseDelay = ldClkPulseDelay;
        if (ldClkPulseDelay > pulseWidth){
            this->ldClkPulseDelay = pulseWidth;
        }
    }

    /**
     * @return Raw input data
     */
    uint64_t GetRawData(){ return inputData; }

    /**
     * @brief Reads data from shift register. 
     * Should be called in loop.
     */
    void ReadData();

    /**
     * @brief Reads data from desired input
     * @returns Input data
     */
    bool GetInputData(uint8_t num){
        return ((bool)(inputData & (1 << num))) == inputLogic;
    }

};

#endif