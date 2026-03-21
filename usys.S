#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(open)
SYSCALL(write)
SYSCALL(close)
SYSCALL(exec)
SYSCALL(uptime)
SYSCALL(sleep)
SYSCALL(getpid)
