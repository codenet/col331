# Initial process execs /init.
# This code runs in user space.

# movl %eax, 0x10001
# movl %eax, 0x1000100
.globl start
start:
  add $1, %eax
  movl %eax, 0x10001
  jmp start