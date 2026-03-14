#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(void) {
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  open("console", O_RDWR);
  open("console", O_RDWR);

  printf(1, "init: starting sh\n");

  for(;;){
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      char *argv[] = { "sh", 0 };
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    // Wait for the shell to exit, then respawn it!
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}