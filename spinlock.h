// Mutual exclusion lock.
#include "types.h"
struct spinlock {
  uint locked;       // Is the lock held?
  char *name;        // Name of lock.
};