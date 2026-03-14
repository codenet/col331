#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(write)
SYSCALL(close)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(fork)
SYSCALL(wait)
SYSCALL(exit)
SYSCALL(getpid)
SYSCALL(kill)
SYSCALL(read)
SYSCALL(dup)
SYSCALL(fstat)
SYSCALL(stat)
SYSCALL(chdir)
SYSCALL(pipe)
SYSCALL(mknod)
