#include "types.h"
#include "defs.h"
#include "x86.h"
#include "fs.h"
#include "buf.h"
#include "param.h"
#include "stat.h"
#include "file.h"

extern char end[]; // first address after kernel loaded from ELF file

static inline void
welcome(void) {
  struct inode* root  = namei("/");
  iread(root);
  struct inode* foodir;

  if((foodir = dirlookup(root, "foo", 0)) == 0) {
    cprintf("/foo not found. Creating!\n");
    foodir = ialloc(ROOTDEV, T_DIR);
    iread(foodir);
    dirlink(foodir, ".", foodir->inum);
    dirlink(foodir, "..", foodir->inum);
    dirlink(root, "foo", foodir->inum);
  }
  
  // Read welcome.txt
  struct inode* wtxt;
  if((wtxt=namei("/foo/greet.txt")) == 0) {
    cprintf("/foo/greet.txt not found. Creating!\n");
    struct inode* wtxt_orig = namei("/welcome.txt");
    dirlink(foodir, "greet.txt", wtxt_orig->inum);
    wtxt=namei("/foo/greet.txt");
  }

  iread(wtxt);
  struct stat st;
  stati(wtxt, &st);

  char greet[512];
  int n = readi(wtxt, greet, 0, st.size);
  cprintf("Read %d bytes from /foo/greet.txt\n", n);
  cprintf("%s\n", greet);
}

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  uartinit();      // serial port
  ideinit();       // disk 
  tvinit();        // trap vectors
  binit();         // buffer cache
  idtinit();       // load idt register
  sti();           // enable interrupts
  iinit(ROOTDEV);  // Read superblock to start reading inodes
  welcome();       // print welcome message
  for(;;)
    wfi();
}
