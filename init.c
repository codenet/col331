#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(void) {
  int p[2];
  char buf[32];

  open("console", O_RDWR); // 0
  open("console", O_RDWR); // 1
  open("console", O_RDWR); // 2

  pipe(p);
  if(fork() == 0) {
    write(p[1], "Pipes are working!\n", 19);
    exit();
  } else {
    wait();
    read(p[0], buf, 19);
    write(1, buf, 19);
  }
  for(;;);
}