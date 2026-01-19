#include "types.h"
#include "defs.h"
#include "x86.h"

extern char end[]; // first address after kernel loaded from ELF file

void
halt(void)
{
  cprintf("Bye COL%d!\n\0", 331);
  outw(0x604, 0x2000);
  // For older versions of QEMU, 
  outw(0xB004, 0x2000);
}


// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  uartinit();      // serial port
  cprintf("Finished setting up PICs!\n\0");
  halt();
}
