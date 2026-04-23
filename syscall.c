#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
// Under paging, p->pgdir is loaded in CR3 during syscall handling, so
// kernel can dereference user virts directly (they're PTE_U, kernel-readable).
// The bound check is now against the process's mapped size, not the old
// segmentation fixed-offset window.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();

  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  int l = fetchstr(addr, pp);
  return l;
}

extern int sys_close(void);
extern int sys_open(void);
extern int sys_write(void);
extern int sys_exec(void);
extern int sys_uptime(void);
extern int sys_sleep(void);
extern int sys_getpid(void);
extern int sys_mknod(void);
extern int sys_fork(void);
extern int sys_wait(void);
extern int sys_exit(void);
extern int sys_kill(void);
extern int sys_read(void);
extern int sys_pipe(void);
extern int sys_dup(void);
extern int sys_sbrk(void);

static int (*syscalls[])(void) = {
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_close]   sys_close,
[SYS_exec]    sys_exec,
[SYS_uptime]  sys_uptime,
[SYS_sleep]   sys_sleep,
[SYS_getpid]  sys_getpid,
[SYS_mknod]   sys_mknod,
[SYS_fork]    sys_fork,
[SYS_wait]    sys_wait,
[SYS_exit]    sys_exit,
[SYS_kill]    sys_kill,
[SYS_read]    sys_read,
[SYS_pipe]    sys_pipe,
[SYS_dup]     sys_dup,
[SYS_sbrk]    sys_sbrk,
};

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}

int
sys_fork(void)
{
  return fork();
}

int
sys_wait(void)
{
  return wait();
}

int
sys_exit(void)
{
  exit();
  return 0;  // Not reached
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}