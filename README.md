# ShiftRegisterPISO
Arduino library for controlling PISO shift register. All library functions
use timestamps instead of sleep() methods. Therefore, reading process is asynchronous and does not block thread execution. 

 - To initialize the reading process, *Init()* method must be called. This method
initializes the pins connected to ShiftRegister and sets the polarity of
signals, as well as number of register inputs.

- For continuous reading, call *ReadData()* method in main program loop.
- To obtain data from specific input, use *GetInputData()* method.

Library further provides methods for setting delays between reading or
adjusting width of single pulse. The real time intervals may depend on computational complexity of the main thread.

To improve input reliability, *SetGlitchPrevention()* method can be used.
Using this option, input data are updated only if the input has been constant for certain number of reading.

Some registers require additional clock edge during loading phase to load inputs. In this case, *SetLdClkPulseDelay()* must be called to enable nested clock pulse during input loading, where *ldClkPulseDelay* is the delay between the edge of loading signal and clock.

