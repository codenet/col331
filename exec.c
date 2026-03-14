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
  uint argc, sp, sz, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proc *curproc = myproc();
  struct proghdr ph;

  char argbuf[512];
  int arg_offsets[MAXARG];
  int argbuf_idx = 0;

  for(argc = 0; argv[argc]; argc++) {
    int len;
    if(argc >= MAXARG)
      return -1;
    len = strlen(argv[argc]) + 1;
    if(argbuf_idx + len > sizeof(argbuf))
      return -1;
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

  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  memset((void*)curproc->offset, 0, PGSIZE - KSTACKSIZE);

  sz = 0;
  for(i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)){
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
    if(ph.vaddr + ph.memsz > PGSIZE - KSTACKSIZE)
      goto bad;

    if(readi(ip, (char*)(curproc->offset + ph.vaddr), ph.off, ph.filesz) != ph.filesz)
      goto bad;
    if(ph.memsz > ph.filesz)
      memset((void*)(curproc->offset + ph.vaddr + ph.filesz), 0,
             ph.memsz - ph.filesz);

    if(ph.vaddr + ph.memsz > sz)
      sz = ph.vaddr + ph.memsz;
  }

  iunlockput(ip);
  end_op();
  ip = 0;

  curproc->sz = sz;
  sp = PGSIZE - KSTACKSIZE;

  for(i = argc - 1; i >= 0; i--) {
    char *arg_str = argbuf + arg_offsets[i];
    sp -= strlen(arg_str) + 1;
    sp &= ~3;
    if(sp < curproc->sz)
      goto bad;
    memmove((void*)(curproc->offset + sp), arg_str, strlen(arg_str) + 1);
    ustack[3+i] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;
  ustack[1] = argc;
  ustack[2] = sp - (argc + 1) * 4;

  sp -= (argc + 1) * 4;
  if(sp < curproc->sz)
    goto bad;
  memmove((void*)(curproc->offset + sp), ustack + 3, (argc + 1) * 4);

  sp -= 12;
  if(sp < curproc->sz)
    goto bad;
  memmove((void*)(curproc->offset + sp), ustack, 12);

  curproc->tf->eip = elf.entry;
  curproc->tf->esp = sp;

  {
    char *last, *s;
    for(last = s = path; *s; s++)
      if(*s == '/')
        last = s + 1;
    safestrcpy(curproc->name, last, sizeof(curproc->name));
  }

  return 0;

bad:
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}