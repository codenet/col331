## System calls

We were able to isolate our processes from each other and disallowed them from
directly using the devices. This brings good protection but it also limits what
processes are able to do.

In this part, we will start making our processes more interesting. If we run the
OS, we see that our processes are able to print something to the console. This
is implemented using *system calls*. Essentially, system calls allow processes
to ask the OS to do some work on their behalf. System calls reuse the hardware's
trap handling mechanism. Previously, when an hardware interrupt (like a timer
interrupt) occured, the hardware was saving all the registers, moving from ring
3 to ring 0, and then giving control to the OS. OS would handle the interrupt,
restore the registers, and give control back to the process.

System calls reuse this functionality but with the only difference that the
traps are now software-generated using the `int` instruction.  We can first see 
the `initcode.S` code which first calls an `open` system call to open the
console device in write only mode, then it calls `write` system call to write a
string to the device, and finally calls `close` system call to close the file. 
Note that the user process pushes the arguments of the system call on to its
stack, sets the type of system call in its `eax` register, and then executes the
`int 64` instruction.

To support system calls, the OS creates a special trap vector at number 64 (in
`traps.h`). This number can be anything but it should not collide with any
hardware generated trap vector. The OS allows ring 3 to generate software 
interrupts for number 64 by setting the Descriptor privilege level to 3 for 
syscalls in `tvinit`. Without this line, the hardware will instead generate a
general protection fault if the process executes `int 64`. The OS will generally 
kill the process upon seeing a general protection fault. Explicitly setting DPL
in IDT is a protection mechanism so that processes cannot pretend to be hardware
devices and call `int` instruction with their trap vectors.

Trap handler in `trap.c` hands over to `syscall` which reads process' `eax`
register from the trapframe to figure out which system call is the process
making. It maintains a list of system call handlers as function pointers in its
`syscalls` table and calls the appropriate system call handler.

System call handlers call methods like `argint` and `argstr` to read call
arguments from user's stack. `argint` assumes that each argument is 32-bit
(4 bytes long): it looks up process' stack pointer in the trapframe, adds four
to ignore the return address, then adds the argument number
* 4 to locate the position of the argument. `fetchint` verifies that the read is
happening within the bounds of the process. This is important as otherwise the
user process could set its `esp` register to arbitrary locations in memory and
then make the system call. `fetchint` also adds process' offset to mimic the
memory segment behavior seen by the process.

`argstr` works in a similar manner: it first reads pointer to the string and
then goes and reads the string.