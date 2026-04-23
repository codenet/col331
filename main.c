#include "types.h"
#include "defs.h"
#include "x86.h"
#include "sleeplock.h"  
#include "fs.h"
#include "buf.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "memlayout.h"


extern char end[]; // first address after kernel loaded from ELF file

// KINIT1 covers this much memory — must fit inside entrypgdir's high-half
// mapping below. xv6-public uses 4MB because kalloc there hands out 4KB
// pages; in this variant kalloc returns 1MB chunks, so 4MB yields only
// ~2 usable pages which is not enough for setupkvm to build kpgdir.
// 128MB (32 PSE PDEs) gives setupkvm plenty of room.
#define KINIT1_MEM  (128*1024*1024)

// Bootstrap page directory. Used only by entry.S before kvmalloc() runs.
// Maps VA [0, 4MB) -> PA [0, 4MB) so entry.S can keep fetching low-half
// instructions, plus VA [KERNBASE, KERNBASE+KINIT1_MEM) -> PA [0, KINIT1_MEM)
// so kinit1 can freerange that region. All PSE 4MB pages.
__attribute__((__aligned__(4096)))
pde_t entrypgdir[NPDENTRIES] = {
  [0]                       = (0) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  0] = (0x00000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  1] = (0x00400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  2] = (0x00800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  3] = (0x00C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  4] = (0x01000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  5] = (0x01400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  6] = (0x01800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  7] = (0x01C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  8] = (0x02000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) +  9] = (0x02400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 10] = (0x02800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 11] = (0x02C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 12] = (0x03000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 13] = (0x03400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 14] = (0x03800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 15] = (0x03C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 16] = (0x04000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 17] = (0x04400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 18] = (0x04800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 19] = (0x04C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 20] = (0x05000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 21] = (0x05400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 22] = (0x05800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 23] = (0x05C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 24] = (0x06000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 25] = (0x06400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 26] = (0x06800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 27] = (0x06C00000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 28] = (0x07000000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 29] = (0x07400000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 30] = (0x07800000) | PTE_P | PTE_W | PTE_PS,
  [(KERNBASE>>PDXSHIFT) + 31] = (0x07C00000) | PTE_P | PTE_W | PTE_PS,
};

// Bootstrap processor starts running C code here.
int
main(void)
{
  kinit1(end, P2V(KINIT1_MEM));           // phys pages inside entrypgdir's window
  consoleinit();                          // (moved up so early panics are visible)
  uartinit();
  kvmalloc();                             // build kpgdir covering all PHYSTOP + devices
  kinit2(P2V(KINIT1_MEM), P2V(PHYSTOP));  // now safe to freerange the rest
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  ideinit();       // disk
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  idtinit();       // load idt register
  sti();           // enable interrupts
  seginit();       // segment descriptors
  pinit();         // first process
  scheduler();     // start running processes
}
