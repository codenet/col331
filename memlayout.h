// Memory layout
// We assume that kernel.asm can fit in first 2MB
#define STARTPROC  0x200000         // Start allocating process from here
#define PROCSIZE  0x100000         // Size of each process