#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  int i, off;
  uint argc, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proc *curproc = myproc();
  struct proghdr ph;
  
  // Use a flat buffer to save kernel stack space! (No more stack overflows)
  char argbuf[512];
  int arg_offsets[MAXARG];
  int argbuf_idx = 0;
  
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG) return -1;
    int len = strlen(argv[argc]) + 1;
    if(argbuf_idx + len > sizeof(argbuf)) return -1;
    memmove(argbuf + argbuf_idx, argv[argc], len);
    arg_offsets[argc] = argbuf_idx;
    argbuf_idx += len;
  }

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  // Wipe old process memory to give the new program a clean slate
  memset((void*)curproc->offset, 1, curproc->sz);

  // Load program into memory
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(readi(ip, (char*)(curproc->offset + ph.vaddr), ph.off, ph.filesz) != ph.filesz)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Set up the user stack
  sp = curproc->sz;
  
  // Push the actual argument strings onto the stack
  for(i = argc - 1; i >= 0; i--) {
    char *arg_str = argbuf + arg_offsets[i];
    sp -= strlen(arg_str) + 1;
    sp &= ~3; // Align to word boundary for x86
    memmove((void*)(curproc->offset + sp), arg_str, strlen(arg_str) + 1);
    ustack[3+i] = sp; 
  }
  ustack[3+argc] = 0; // null-terminate the argv array

  // Push the argv pointer array, argc, and the fake return PC
  ustack[0] = 0xffffffff; // Fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc + 1) * 4; // Memory address where the argv array will sit

  // Push argv pointers array
  sp -= (argc + 1) * 4;
  memmove((void*)(curproc->offset + sp), ustack + 3, (argc + 1) * 4); 
  
  // Push the Fake PC, argc, and argv pointer
  sp -= 12; 
  memmove((void*)(curproc->offset + sp), ustack, 12); 

  curproc->tf->eip = elf.entry;  // Start exactly at 'main'
  curproc->tf->esp = sp;         // Set the stack pointer
  
  // Update the process name for debugging
  char *last, *s;
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));
  
  return 0;

 bad:
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}