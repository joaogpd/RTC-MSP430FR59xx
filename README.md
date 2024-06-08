Hardware abstraction layer (HAL) for accessing the RTC_B module in the MSP430FR59xx family of boards, by Texas Instruments.

There are commodity functions for enabling interrupts, configuring the RTC for operation, etc. The abstraction offers a callback that can be inserted into the ISR when it's called, but that needs to be user defined.

One example is offered, which also makes use of the ADC abstraction located in [this repository](https://github.com/joaogpd/ADC-MSP430FR59xx) for reading the temperature from an analog sensor and printing the value to standard output every minute.