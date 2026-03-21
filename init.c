// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd = open("console", O_RDWR);
  int pid = getpid();
  printf(0, "[%d] Hello %s from init.c\n", pid, argv[0]);
  while(1) {
    int t = uptime();
    printf(fd, "[%d] It's %d ticks since start.\n", pid, t);
    sleep(100);
  }
}
