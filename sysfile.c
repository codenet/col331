//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0) {
    return -1;
  }
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0) {
    return -1;
  }
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp); // <-- Lock directory

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip); // <-- Lock target file
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip); // <-- Lock new file
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  
    dp->nlink++;  
    iupdate(dp);
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip; // Returns with ip still locked!
}

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if((argstr(0, &path) < 0) || (argint(1, &omode) < 0)) return -1;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip); // <-- Lock opened file
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f) fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  
  iunlock(ip); // <-- Done setting up, safely unlock
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)) return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0) return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0) return -1;
  }
  
  return exec(path, argv);
}

int sys_dup(void) {
  struct file *f;
  int fd;
  if(argfd(0, 0, &f) < 0) return -1;
  if((fd = fdalloc(f)) < 0) return -1;
  filedup(f);
  return fd;
}

int sys_fstat(void) {
  struct file *f;
  struct stat *st;
  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0) return -1;
  return filestat(f, st);
}

int sys_stat(void) {
  struct inode *ip;
  struct stat *st;
  char *path;
  if(argstr(0, &path) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0) return -1;
  begin_op();
  if((ip = namei(path)) == 0){ end_op(); return -1; }
  ilock(ip);
  stati(ip, st);
  iunlockput(ip);
  end_op();
  return 0;
}

int sys_chdir(void) {
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0) return -1;
  ilock(ip);
  if(ip->type != T_DIR) { iunlockput(ip); return -1; }
  iunlock(ip);
  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int sys_pipe(void) {
  return -1; // Dummy return for p25. Pipes are added in p26!
}

int sys_mknod(void) {
  struct inode *ip;
  char *path;
  int major, minor;

  if((argstr(0, &path) < 0) ||
     (argint(1, &major) < 0) ||
     (argint(2, &minor) < 0)){
    return -1;
  }
  
  begin_op();
  if((ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  
  return 0;
}