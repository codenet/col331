#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  pushcli();
  while (lk->locked) {
    popcli();
    sleep(lk);
    pushcli();
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  popcli();
}

void
releasesleep(struct sleeplock *lk)
{
  pushcli();
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  popcli();
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  pushcli();
  r = lk->locked && (lk->pid == myproc()->pid);
  popcli();
  return r;
}