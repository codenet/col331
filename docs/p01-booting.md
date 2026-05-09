# Appendix B

## Booting sequence:

`BIOS -> bootasm.S -> bootmain.c -> entry.S -> main.c`

BIOS does initial hardware check: are the CPUs, memory, disk etc functional? 
Then it loads the first sector (first 512 bytes) from the disk into memory
address `0x7c00` and sets processor's instruction pointer to jump to this memory
address. This first sector is called the "bootloader". It is a simple program
that fits in just 512 bytes. Its goal is simple: load more sectors from the disk
and jump to that. 

For xv6, bootloader will jump directly to the OS. For other systems, we might
want to first figure out which OS to run, download that OS from the network,
etc. In such cases, bootloader might jump to a small OS that can drive display,
use the keyboard, download things from the network etc to let users download the
OS.  This small OS then gives control to the downloaded OS.

### Bootloader

The first sector of the disk contains `bootblock` (basically bootloader) which
is prepared from `bootasm.S` and `bootmain.c`. This `bootblock` is copied into
`xv6.img`. We first zero 10,000 sectors of `xv6.img` and then write the
`bootblock` to the first sector.

In preparing `bootblock`, we mention that the symbol `start` declared in 
`bootasm.S` starts on memory location `0x7C00`. This is done to make the
physical address and the virtual address identical since we have not yet setup
the virtual memory.

`bootblock.asm` disables interrupts, clears segment registers for keeping
identical virtual and physical addresses, changes from 16-bit mode to 32-bit
mode, and finally jumps into `bootmain.c`. These steps are described as is in
Section 9.9.1, Volume 3A, Part 1 of Intel SDM.

Changing from 16-bit to 32-bit mode changes the code segment register to `0x8`.
All the other segment registers are zero.

`bootmain.c` has to now load the OS from the disk and give control to it. It
first reads the next 4KB from the disk into memory location `0x10000`. The OS is
stored in an *Executable Linux File (ELF) format*. The format specifies a standard
on how an executable is stored so that we can safely give control to it. This
format enables us to move executables from one machine to another (assuming same
ISA etc). Our OS is just another executable.

The ELF file first has a header that should starts with a magic string. This is
a mechanism to check whether the executable is corrupted. If the check passes, 
we load the program into memory by reading more disk blocks and finally jump to
the `entry` point as mentioned in the ELF file.

Open xv6.img in a hex editor and see that the ELF magic word is written at
0x200,00 (byte number 512). At 0x210,08 in xv6.img, we have `0x0C001000` which
is little endian for `0x10000C`. In `kernel.asm`, we can see that `entry` is
located at the address `0x10000c`.

### OS 
`entry.S` finally sets up a 4KB stack for the OS and jumps to `main()` method
defined by `main.c`. `main.c` just exits by sending a special word indicating
shutdown to QEMU.
