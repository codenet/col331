#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H
// Long-term locks for processes

struct sleeplock {
  uint locked;       // Is the lock held?
  struct spinlock lk; // Spinlock protecting this sleep lock
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

#endif