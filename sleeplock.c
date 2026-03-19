#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// #include "spinlock.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  // initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  // acquire(&lk->lk);
  pushcli();
  while (lk->locked) {
    // release(&lk->lk);
    popcli();
    sleep(lk);
    // acquire(&lk->lk);
    pushcli();
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  // release(&lk->lk);
  popcli();
}

void
releasesleep(struct sleeplock *lk)
{
  // acquire(&lk->lk);
  pushcli();
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  // release(&lk->lk);
  popcli();
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  // acquire(&lk->lk);
  pushcli();
  r = lk->locked && (lk->pid == myproc()->pid);
  // release(&lk->lk);
  popcli();
  return r;
}