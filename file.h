struct file {
  enum { FD_NONE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct inode *ip;
  uint off;
};

// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};