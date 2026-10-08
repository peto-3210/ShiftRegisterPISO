## ShiftRegisterPISO
### Description
Arduino library for controlling PISO shift register.
PISO stands for Parallel In Serial Out. This type of shift register is used to read multiple digital inputs using only a few microcontroller pins.

The library relies on timestamps to manage timing of reading and clock signals. This way, it does not block the main program loop, allowing other parts of the code to run concurrently.

Library further provides methods for setting delays between reading or
adjusting frequency of clock signal. The real time intervals may depend on computational complexity of the main thread.

### Usage
The library provides a class *PISORegister* with methods for initializing and reading data from the shift register.

#### Initialization
To initialize the reading process, *Init()* method must be called. This method initializes the pins connected to shift register and sets the polarity of
signals, as well as number of register inputs.
Required inputs:
 - *pinNumber* Number of register inputs (1 to 64, values outside this range are clamped)
 - *clkPin* Pin used for clock signal
 - *ldPin* Pin used for asynchronous load signal
 - *qhPin* Input from shift register
 - *clkPolarity* Polarity of clock edge - true for rising edge, false for falling edge
 - *ldPolarity* Logic level of asynchronous loading - false for 0, true for 1
 - *inputLogic* Type of input logic (true for normal, false for inverse)

#### Additional settings
- *SetFrequency()* - Set frequency of clock signal in Hz (default 5000)
- *SetReadingDelay()* - Set delay between readings in microseconds (default 0)
- *SetGlitchPrevention()* - Set number of consecutive identical readings required before input data is updated (default 1 = no filtering)
- *SetLoadingClockPulse()* - If argument is > 0, generates an additional clock pulse during the loading phase. Some registers require this to load inputs. The value is the gap in microseconds between the loading signal edge and the clock pulse edge, clamped to the clock half-period. Call it *after* *SetFrequency()*, because *SetFrequency()* resets it to 0.

#### Reading data
For continuous reading, call *ReadData()* method in main program loop.

To improve input reliability, *SetGlitchPrevention()* method can be used.
In this case, input data are updated only if the input has been constant for a certain number of readings.

To obtain data from specific input, use *GetInput()* method. This input corresponds to the input number on the shift register (0 to pinNumber-1). Out-of-range numbers return false.

To obtain data from all inputs as a single 64-bit integer, use *GetAllInputData()* method. Note that this returns the raw register value, without *inputLogic* applied.
