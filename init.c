// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd = open("console", O_RDWR);
	int t = uptime();
  printf(fd, "Hello %s from init.c. It's %d ticks since start.\n", argv[0], t);
  close(fd);

  while(1);
}
