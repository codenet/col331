#include "types.h"
#include "x86.h"

extern char end[]; // first address after kernel loaded from ELF file

int
halt(void)
{
  outw(0x604, 0x2000);
  // For older versions of QEMU, 
  outw(0xB004, 0x2000);
  return 0;
}

// Bootstrap processor starts running C code here.
int
main(void)
{
  halt();
}