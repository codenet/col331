#include "types.h"
#include "defs.h"
#include "x86.h"
#include "fs.h"
#include "buf.h"
#include "param.h"
#include "stat.h"
#include "file.h"
#include "fcntl.h"

extern char end[]; // first address after kernel loaded from ELF file

static inline void
welcome(void) {
  // Create and write /foo/hello.txt

  mkdir("/foo");
  struct file* gtxt;
  if((gtxt = open("/foo/hello.txt", O_CREATE | O_WRONLY)) == 0){
    panic("Failed to create /foo/hello.txt");
  }
  int n = filewrite(gtxt, "hello\0", 6);
  cprintf("Wrote %d characters to /foo/hello.txt\n", n);
  fileclose(gtxt);

  if((gtxt=open("/foo/hello.txt", O_RDONLY)) == 0) {
    panic("Unable to open /foo/hello.txt");
  }
  char welcome[512];
  n = fileread(gtxt, welcome, 6);
  cprintf("Read %d chars from /foo/hello.txt: %s\n", n, welcome);
  fileclose(gtxt);

	// Delete /foo/hello.txt
  char name[DIRSIZ];
 
  unlink("/foo/hello.txt", name);
  struct inode* foo = namei("/foo");
  if(!isdirempty(foo)) {
    panic("/foo should be empty");
  }

  if((gtxt = open("/foo/hello.txt", O_RDONLY)) != 0) {
    panic("Could open /foo/hello.txt after unlinking");
  }
  
  // Print welcome message
  struct file* wtxt;
  if((wtxt=open("/welcome.txt", O_RDONLY)) == 0) {
    panic("Unable to open /welcome.txt");
  }
  n = fileread(wtxt, welcome, 512);
  cprintf("Read %d chars from /welcome.txt:\n %s", n, welcome);
  fileclose(wtxt);
}

// Bootstrap processor starts running C code here.
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
  initlog(ROOTDEV);  // Initialize log
  welcome();       // print welcome message
  for(;;)
    wfi();
}
