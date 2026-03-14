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
  char *s, *last;
  int i, off;
  uint argc;
  struct elfhdr elf;
  struct inode *ip;
  struct proc *curproc = myproc();
  struct proghdr ph;
  char *offset;
  uint usp, ustack[3*MAXARG + 1];

  // Prepare new address space 
  if((offset = kalloc()) == 0){
    return -1;
  }
  memset(offset, 0, PGSIZE);

  // read path
  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  // Load program into memory.
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
    if(ph.vaddr + ph.filesz > PGSIZE)
      goto bad;
    if(readi(ip, (char*)(offset + ph.vaddr), ph.off, ph.filesz) != ph.filesz)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Push argument strings, prepare rest of stack in ustack.
  usp = PGSIZE;
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    usp = usp - (strlen(argv[argc]) + 1);
    // cprintf("%s\n", argv[argc]);
    memmove((uint*)(usp + offset), argv[argc], strlen(argv[argc]) + 1);
    ustack[3+argc] = usp;  // Add pointer to the string on the stack
  }
  // cprintf("%d\n", argc);
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = (usp - (argc+1)*4);  // argv pointer
  usp -= (3+argc+1)*4;
  memmove(usp + offset, ustack, (3+argc+1)*4);

  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = usp;

  // We free the old address space. It does not contain the kernel stack! kfree
  // writes 1s to the entire PGSIZE. All the return addresses etc will get
  // messed up, otherwise!
  kfree(curproc->offset);
  curproc->offset = offset;
  switchuvm(curproc);
  return 0;

 bad:
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
