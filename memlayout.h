// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTART (EXTMEM + PROCSIZE) // Start physical memory
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout
#define KERNBASE 0x0         // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked
#define PROCSIZE   0x100            // 1MB is the size of each process (in multiple of 4KB)

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts