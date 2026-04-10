// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
  int fd = open("console", O_RDWR);
	int t = uptime();
  printf(fd, "Hello from init.c. It's %d ticks since start.\n", t);
  close(fd);

  while(1);
}
