//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "file.h"
#include "stat.h"
#include "fcntl.h"

struct devsw devsw[NDEV];
struct {
  struct file file[NFILE];
} ftable;

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      return f;
    }
  }
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;

  if(ff.type == FD_INODE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    iread(f->ip);
    stati(f->ip, st);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_INODE){
    iread(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    return r;
  }
  panic("fileread");
}

// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_INODE){
    // write a few blocks at a time
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

			begin_op();
      iread(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

// Is the directory dp empty except for "." and ".." ?
int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

int
unlink(char* path, char* name)
{
  struct inode *ip, *dp;
  struct dirent de;
  uint off;

	begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  iread(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  iread(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iput(dp);

  ip->nlink--;
  iupdate(ip);
  iput(ip);

  end_op();
  return 0;

bad:
  iput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  iread(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iput(dp);
    iread(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  iread(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iput(dp);

  return ip;
}


struct file*
open(char* path, int omode)
{
  struct inode *ip;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return 0;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return 0;
    }
    iread(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iput(ip);
      end_op();
      return 0;
    }
  }

  struct file* f;
  if((f = filealloc()) == 0) { 
    iput(ip);
    end_op();
    return 0;
  }

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  end_op();
  return f;
}

int
mknod(struct inode *ip, char* path, int major, int minor)
{
  begin_op();
  if((ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iput(ip);
  end_op();
  return 0;
}

int mkdir(char *path)
{
  struct inode *ip;

  begin_op();
  if((ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return -1;
  }
  iput(ip);
  end_op();
  return 0;
}