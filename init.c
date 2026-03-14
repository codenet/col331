// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int main() {
  static char hello[] = "Hello from init.c\n";
  int fd = open("console", O_RDWR);
  if(fd >= 0){
    write(fd, hello, sizeof(hello) - 1);
    close(fd);
  }

  while(1);
}
