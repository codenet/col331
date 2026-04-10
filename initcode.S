# Initial process that runs in user space.

#include "syscall.h"
#include "traps.h"

.globl start
start:
  pushl $0
  pushl $0
  call main
