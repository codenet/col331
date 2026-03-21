#include "types.h"
#include "x86.h"
#include "defs.h"
#include "mmu.h"
#include "proc.h"

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    sleep(&ticks);
  }
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void) {
  return ticks;
}

int
sys_getpid(void)
{
  return myproc()->pid;
}