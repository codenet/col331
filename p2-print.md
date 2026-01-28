Self study the diff:

`git diff p1-booting`


# UART Documentation

UART stands for Universal Asynchronous Receiver/Transmitter. It is an interface for asynchronous (without a synchronizing clock signal) serial communication in PCs.

The uart.c file essentially describe the functions to communicate with the UART interface which in this case is the PC8250A UART(The exact part might be different as different models have been introduced over time, however the basic functionality and register mapping is consistent). 

The PC8250 has 8 programmable registers which are accessed by the processor using port mapped IO. In the case of X86, the location 0x3f8 points to the base register of the UART (Equivalent to the 0 register in the UART datasheet). The pin descriptions given in the datasheet should be read along with the code to understand there function. These registers are written using the outb instructions. The in and out instructions are needed for port mapped IO unlike APICs.

QEMU also simulates the UART. So if we include the code to control the registers of the UART we will see the appropriate output on screen. For example the uartputc function waits until the Transmission Holding Register Empty bit is set which indicates that the UART is ready to transmit another character before writing a character to the data register.

[UART Datasheet](https://sys.cs.fau.de/extern/lehre/ws22/bs/uebung/aufgabe3/uart-8250a.pdf)
