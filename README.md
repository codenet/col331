This branch contains the implementation for handling hardware and software interrupts. To understand the flow from a hardware signal to the kernel's C code, refer to the following key files:

* **[p4-traps.md](p4-traps.md)**: The primary documentation providing a step-by-step technical walkthrough of the trap mechanism, including hardware-to-software transitions.
* **[vectors.pl](vectors.pl)**: A Perl script used to automatically generate the `vectors.S` assembly file. This file creates the 256 entry points (vectors) required by the Interrupt Descriptor Table (IDT).
* **[trap.c](trap.c)**: Contains the C-level logic for initializing the IDT (`tvinit` and `idtinit`) and the high-level trap dispatcher (`trap`) that routes interrupts to their respective handlers.
* **[trapasm.S](trapasm.S)**: Low-level assembly routines that bridge the gap between the raw vector entry and the C `trap` function. It defines `alltraps`, which builds the `trapframe`, and `trapret`, which restores state before returning to user space.



* **[main.c](main.c)**: The kernel bootstrap file. It orchestrates the setup process by calling initialization routines for the interrupt table and finally enabling interrupts on the CPU using the `sti` instruction.