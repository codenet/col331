// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  printf(0, "Hello %s from init.c\n", argv[0]);
  while(1) {
    int t = uptime();
    printf(0, "It's %d ticks since start.\n", t);
    sleep(100);
  }
}