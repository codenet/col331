// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd = open("console", O_RDWR);
  printf(fd, "Hello %s from init.c\n", argv[0]);
  close(fd);

  while(1);
}
