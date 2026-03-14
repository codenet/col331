# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  movl $SYS_exec, %eax
  int $T_SYSCALL

# char *argv[] = { "world", 0 };
argv:
  .long world
  .long 0

# char init[] = "/init\0";
init:
  .string "/init\0"

# char world[] = "world\0";
world:
  .string "world\0"
