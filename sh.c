#include "types.h"
#include "user.h"
#include "fcntl.h"

#define MAXMEM 8192
char sh_memory[MAXMEM];
int sh_mem_idx = 0;

void* malloc(uint size) {
  char *p = &sh_memory[sh_mem_idx];
  sh_mem_idx += ((size + 3) & ~3); // word align
  if (sh_mem_idx > MAXMEM) {
    printf(2, "sh: out of memory\n");
    exit();
  }
  return (void*)p;
}

void free(void *p) { /* Not implemented in static allocator */ }

int getcmd(char *buf, int nbuf) {
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) return -1;
  return 0;
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
      
      // 2. Tokenize the string in-place by turning spaces into null-terminators
      while(*p) {
        while(*p == ' ' || *p == '\t') *p++ = 0; // Skip leading spaces
        if(*p == 0) break;
        
        argv[argc++] = p; // Record the start of the word
        while(*p && *p != ' ' && *p != '\t') p++; // Skip to the end of the word
      }
      argv[argc] = 0;

      if(argc == 0) exit();

      // 3. Check for I/O Redirection (expects spaces, e.g., "echo a > b")
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

      // Apply redirections
      if(redir_in) {
        close(0);
        if(open(redir_in, O_RDONLY) < 0) { printf(2, "cannot open %s\n", redir_in); exit(); }
      }
      if(redir_out) {
        close(1);
        if(open(redir_out, O_WRONLY|O_CREATE) < 0) { printf(2, "cannot create %s\n", redir_out); exit(); }
      }

      // Execute the isolated command
      exec(exec_argv[0], exec_argv);
      
      // If exec returns, it failed
      printf(2, "exec %s failed\n", exec_argv[0]);
      exit();
    }
    wait();
  }
  exit();
}