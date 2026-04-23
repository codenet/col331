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

// Bootstrap page directory. Used only by entry.S before kvmalloc() runs.
// Maps VA [0, 4MB) -> PA [0, 4MB) so entry.S can keep fetching low-half
// instructions, plus VA [KERNBASE, KERNBASE+4MB) -> PA [0, 4MB) so the
// kernel can run at its linked high-half virtual addresses. Both 4MB PSE.
__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0]                     = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT]    = (0) | PTE_P | PTE_W | PTE_PS,
};

// Bootstrap processor starts running C code here.
int
main(void)
{
  kinit1(end, P2V(4*1024*1024));          // phys pages inside entrypgdir's window
  consoleinit();                          // (moved up so early panics are visible)
  uartinit();
  kvmalloc();                             // build kpgdir covering all PHYSTOP + devices
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // now safe to freerange the rest
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
