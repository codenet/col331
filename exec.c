#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path)
{
  int i, off;
  uint sp;
  struct elfhdr elf;
  struct inode *ip;
  struct proc *curproc = myproc();
  struct proghdr ph;

  begin_op();

  // cprintf("exec: path %s\n", path);
  if((ip = namei(path)) == 0){
    end_op();
    // cprintf("exec: fail\n");
    return -1;
  }
  iread(ip);

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  // Fill with junk
  memset(curproc->offset, 1, curproc->sz);

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
    if(readi(ip, (char*)(curproc->offset + ph.vaddr), ph.off, ph.filesz) != ph.filesz)
      goto bad;
  }
  iput(ip);
  end_op();
  ip = 0;

  sp = curproc->sz;
  *(uint*)sp = 0xffffffff;
  sp -= 4;

  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  return 0;

 bad:
  if(ip){
    iput(ip);
    end_op();
  }
  return -1;
}
