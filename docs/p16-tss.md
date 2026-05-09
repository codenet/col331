## Devices and processes

### Handling interrupts while a process is running

The previous part did not handle interrupts. Before running the task, we had
changed the stack segment `ss` (in `pinit`). But when interrupt happens, the
processor does not want to use the same stack segment as then it will be writing
"OS level" information like the interrupt number into the process' address
space. This could be dangerous. For instance, if the OS were running on multiple
CPUs, the same process could also be running on the other CPU. The process could
then read or write this memory region from the other CPU. (We will see how
multiple CPUs work in xv6 later.)

Therefore, the hardware needs to know the stack segment it can use when it needs
to switch to privilege level zero i.e, at the time of interrupt. The hardware
reads this stack segment from its current "task state segment".

Task state segments (TSS) are described in Chapter 7 of Intel SDM Volume 3A.
Before running `initproc`, `switchuvm` in `vm.c` prepares a `tss` and sets its
`ss0` to the kernel's data segment. This `tss` is loaded onto the processor by
calling `ltr` (load task register) instruction, similar to the `lgdt` and `lidt`
instructions we have seen earlier.

Now within the TSS, we further specify `esp0` to point to process's *kernel
stack*. This kernel stack is created in `allocproc`: last 4KB out of 1MB of
process' address space is given to kernel stack. All the OS data structures like
trapframes should be written into this kernel stack area. We also ensure in
`seginit` in `vm.c` that the last 4KB occupied by the kernel stack is not mapped
into the process' address space.

After the OS finishes booting, press "Ctrl+P" to see the running process.

### Disallowing process to interact with devices

By limiting the addressable memory of the process, we removed the process'
ability to do memory mapped IO. The process will not be able to "address" the 
memory that is mapped to IO operations. But we have not yet protected port based
IO.  For instance, the process might be able to directly read and write to disk,
bypassing the filesystem, by calling the `in` and `out` instructions such as the
ones called by `ide.c`.

Note that the hardware provides the ability to let processes directly read and
write harmless IO ports. For example, a user process might want to directly
control some LEDs without having to switch to privilege level zero. An OS can
enable this by setting `IOPL=3` in the `eflags` register of the process' task
state segment indicating that we are allowing the low privilege process to
perform IO. The `eflags` register is described in Section 3.4.3 of Intel SDM
Volume 1.

But, we might want to selectively give access to some ports (such as only a few
LEDs but not the whole disk). The OS can further set the "I/O permission bit map"
in the process' task state segment (TSS), described in Section 18.5.2 in Intel
SDM, Volume 1.

In xv6, we don't want to do all that.  We request the processor to trap on IO
instructions by setting `iopl` bit to zero in `eflags` in `pinit`. We also set
`iomb` in `switchuvm` to all 1s. This essentially informs the hardware to raise
a fault if the user program tries to call `in` or `out` instructions. This
protects devices from the processes. 