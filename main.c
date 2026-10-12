#include "types.h"
#include "defs.h"
#include "x86.h"
#include "fs.h"
#include "buf.h"
#include "param.h"
#include "stat.h"
#include "file.h"
#include "fcntl.h"
#include "mmu.h"
#include "proc.h"
#include "memlayout.h"

extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
int
main(void)
{
  kinit(end, P2V(PHYSTOP)); // phys page allocator
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  ideinit();       // disk 
  tvinit();        // trap vectors
  binit();         // buffer cache
  idtinit();       // load idt register
  sti();           // enable interrupts
  iinit(ROOTDEV);  // Read superblock to start reading inodes
  initlog(ROOTDEV);  // Initialize log

  struct inode console;
  mknod(&console, "console", CONSOLE, CONSOLE);
  seginit();       // segment descriptors
  pinit();         // first process
  pinit();         // another process
  scheduler();     // start running processes
}
