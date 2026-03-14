#include "types.h"
#include "user.h"
#include "fcntl.h"

int getcmd(char *buf, int nbuf) {
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) return -1;
  return 0;
}

// Helper function to handle redirection and execution
void runcmd(char **argv, int argc) {
  char *redir_in = 0;
  char *redir_out = 0;
  char *exec_argv[20];
  int exec_argc = 0;

  for(int i = 0; i < argc; i++) {
    if(strcmp(argv[i], "<") == 0 && i + 1 < argc) {
      redir_in = argv[++i];
    } else if(strcmp(argv[i], ">") == 0 && i + 1 < argc) {
      redir_out = argv[++i];
    } else {
      exec_argv[exec_argc++] = argv[i];
    }
  }
  exec_argv[exec_argc] = 0;

  if(redir_in) {
    close(0);
    if(open(redir_in, O_RDONLY) < 0) { printf(2, "cannot open %s\n", redir_in); exit(); }
  }
  if(redir_out) {
    close(1);
    if(open(redir_out, O_WRONLY|O_CREATE) < 0) { printf(2, "cannot create %s\n", redir_out); exit(); }
  }

  exec(exec_argv[0], exec_argv);
  printf(2, "exec %s failed\n", exec_argv[0]);
  exit();
}

int main(void) {
  static char buf[128];
  int fd;

  // Ensure 0, 1, 2 are allocated to the console
  while((fd = open("console", O_RDWR)) >= 0) {
    if(fd >= 3) { close(fd); break; }
  }

  while(getcmd(buf, sizeof(buf)) >= 0) {
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
      buf[strlen(buf)-1] = 0; 
      if(chdir(buf+3) < 0) printf(2, "cannot cd %s\n", buf+3);
      continue;
    }

    if(fork() == 0) {
      char *argv[20];
      int argc = 0;
      char *p = buf;
      
      // 1. Remove the trailing newline character
      int len = strlen(buf);
      if(len > 0 && buf[len-1] == '\n') buf[len-1] = 0;
      
      // 2. Tokenize the string
      while(*p) {
        while(*p == ' ' || *p == '\t') *p++ = 0;
        if(*p == 0) break;
        
        argv[argc++] = p;
        while(*p && *p != ' ' && *p != '\t') p++;
      }
      argv[argc] = 0;

      if(argc == 0) exit();

      // 3. Check for Pipe character '|'
      int pipe_idx = -1;
      for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "|") == 0) {
          pipe_idx = i;
          break;
        }
      }

      if(pipe_idx != -1) {
        int p[2];
        pipe(p); // Create the kernel pipe!
        
        if(fork() == 0) {
          // Left side of the pipe
          close(1);   // Close standard output
          dup(p[1]);  // Wire standard output to the pipe's write end
          close(p[0]);
          close(p[1]);
          runcmd(argv, pipe_idx);
        }
        if(fork() == 0) {
          // Right side of the pipe
          close(0);   // Close standard input
          dup(p[0]);  // Wire standard input to the pipe's read end
          close(p[0]);
          close(p[1]);
          runcmd(&argv[pipe_idx + 1], argc - pipe_idx - 1);
        }
        close(p[0]);
        close(p[1]);
        wait();
        wait();
        exit();
      } else {
        // No pipe, run normally
        runcmd(argv, argc);
      }
    }
    wait();
  }
  exit();
}