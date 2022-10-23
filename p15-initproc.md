## Our first process!

The OS is now going to run its first process: an infinite loop defined in
`initcode.S`. This is one of the most phenomenal parts of an operating system.
We want to give full control to the process i.e, invoke minimal performance
overhead while the process is running, but want to keep control of the hardware,
i.e, protect OS from the process and processes from each other. Doing all this
is difficult for an OS without getting assistance from the hardware. Let us see
how the OS and the hardware work together to achieve this.

### Virtual address space

The first part of isolation is keeping control of the memory. With the address
translation mechanism provided by memory segments (described in the last part),
we limit the memory visible to the process. Instead of letting the process read
and write all of 4GB of physical memory, the process will be limited to its own
memory segments from 2MB to 3MB. We had setup these segments in `vm.c`.

Therefore, when the process reads or writes address `0x10001`, the process will
actually read or write to the address `0x10001 + 0x200000`, where `0x200000` is
`STARTPROC` defined in `memlayout.h`.  We can check this out by doing `Ctrl+C`
in `qemu` and inspect what is in `0x210000`.

```
(gdb) x /8xb 0x210000
0x210000:       0x00    0x92    0xe3    0xf4    0x82    0x00    0x00    0x00
```

By creating such memory segments, we can isolate the process from the OS and
processes from one another. For example, the process will not be able to overwrite
the OS's key data structures such as the global descriptor table (GDT) and the
interrupt descriptor table (IDT). Similarly, the process will not be able to
modify the code of the OS, since they fall out of the memory segments
addressable by the process.

This addressable area is also called the *address space* of the process. A
process can read and write its own address space. Physical memory regions may or
may not be in the address space of the process. If the process tries to use
locations beyond its segment's limit, the hardware generates a *segmentation
fault*.

> Note: Writing to memory locations beyond the segment limits in `initcode.S` is
not generating a segfault. Debug why not. The write does not succeed as checked
with `x /8xb 0x1200100` in gdb. It probably has something to do with auto
expansion of segments described in Section 3.4.5 of Intel SDM Volume 3A.

In addition to isolation, virtualizing memory allows processes to be placed
*anywhere* in physical memory. From the point of view of the process, the
process starts at the location zero, and can read and write (almost) all the
memory locations. We see in `pinit` in `proc.c` that the OS just copies the
binary of `initcode` to a physical memory location `STARTPROC` of its choice,
sets the memory segments for the process starting at `STARTPROC`, and sets `eip`
to **0**. `eip=0` will get translated to `0x200000` when the process starts with
its code segment selector base set as `STARTPROC`.

### Privilege levels a.k.a. rings

Virtualizing memory only partly solves the isolation problem. What stops a
process from calling instructions like `lgdt` and `lidt` to take control of the
memory segments and the interrupts. If a process wants to illegally overwrite OS
code, it can setup its own GDT by calling `lgdt` like in `seginit` in `vm.c` to
give itself permission to access other parts of memory.

This is where *privilege levels* come into play. See Figure 5.3 in Intel SDM
Volume 3A. x86 supports 4 privilege levels. Typically, levels 1 and 2 are
unused.  OS runs in level 0 and user programs run in level 3. Smaller ring
number means that we have more privilege.

Hardware determines the *current privilege level (CPL)* by the least significant
two bits of the code selector register. Similarly, least significant two bits of
data segments define *required privilege level (RPL)* for the segment. This
means that this data segment can be read/written by code running at a privilege
level of RPL or higher (i.e, with CPL <= RPL). Similarly, *descriptor privilege
level (DPL)* is also set in segment descriptor set in GDT. Note how `pinit`
initializes segment registers' last two bits with `DPL_USER`(= 3). `vm.c` also sets
segment descriptor entries to `DPL_USER`.

With all this, hardware prevents a user program running at privilege level=3
switch its code segment to privilege level=0.  User programs are also *not
allowed* to change GDT, IDT, etc. as they are *privileged instructions* (list of
privileged instructions is given in Section 5.9 Intel SDM Volume 3A). If a user
program tries to run these instructions a general protection fault is raised by
the hardware which gives control to the OS. OS would typically just kill the
offending process.

> We would have liked to describe process creation without talking about
privilege levels, and then later describe privilege levels, but x86 does not
switch the stack segment if the process and the OS are running in the same
privilege level. Therefore, we were forced to talk about levels in this part.

### Putting it together

To put it all together, `main.c` first calls `seginit` to set two new segments
for our first process. Then it calls `pinit` which calls `allocproc` to allocate
an `UNUSED` process marking it as `EMBRYO` signifying that this process is being
created. 

`allocproc` creates a stack pointer to point to the end of the processes' memory
segment. It leaves space for storing the `trapframe` and sets `eip` to
`trapret`. `allocproc` wants us to pretend like we are returning from a trap.
`trapret` will later read the `trapframe` and give control to the process.

After the process is allocated, `pinit` copies the program binary starting at
`STARTPROC`, saves memory segment registers in the trapframe for the embryo
process, sets `eip` to zero which will be at the start of `initcode`, and marks
the process as `RUNNABLE`. This symbolizes that the process is now ready to run.

`main.c` finally calls `scheduler` in `proc.c` which looks for a `RUNNABLE`
process and switches to it. In x86, caller already saves some registers, e.g:
eax, etc. on the stack. `swtch.S` saves rest of the "callee-saved registers" on
the stack, switch the stack to the stack of our new process, and restores all
the same set of "callee-saved registers". Calling `ret` sets `eip` to the return
address saved on the stack which `allocproc` had initialized to `trapret`.
Therefore, we jump into `trapret` in `trapasm.S`.

`trapret` thinks that we are just returning from servicing a trap. It pops
segment registers from the `trapframe` and runs `iret`. We had set segment
registers to the users' memory segments in `pinit`. `iret` restores the rest of
the registers (most of which are just set to zero) and also change the stack
segment, code segment, and `eip`. Since, `eip` was set to zero in our fake
`trapframe` constructed by `pinit`, the CPU finally jumps to the start of
`initcode.S`.

> Note that we have not yet setup capability to safely get the control back from
the process. If you press a key in the console after the process has started
running, we will see that the OS crashes. We will add this capability in the 
next part.