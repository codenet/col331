// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout
#define KERNBASE 0x0         				// First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

// We assume that kernel.asm can fit in first 2MB
#define STARTPROC  0x200000         // Start allocating process from here
#define PROCSIZE  0x100000         // Size of each process