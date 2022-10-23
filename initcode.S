# Initial process that runs in user space.

#include "syscall.h"
#include "traps.h"

# sys_open("console", O_WRONLY)
pushl $1
pushl $console
pushl $0
movl $SYS_open, %eax
int $T_SYSCALL

# return fd in eax

# sys_write(fd, "hello world\n\0")
pushl $hello
pushl %eax
pushl $0
movl $SYS_write, %eax
int $T_SYSCALL

movl $SYS_close, %eax
int $T_SYSCALL

.globl start
start:
  ; add $1, %eax
  ; movl %eax, 0x10001
  jmp start

console:
  .string "console\0"

hello:
  .string "hello world from inside the process!\n\0"