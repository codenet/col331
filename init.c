// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
  int fd = open("console", O_RDWR);
  printf(fd, "Hello COL%d from init.c!\n", 331);
  close(fd);

  // exec("/init");
}
