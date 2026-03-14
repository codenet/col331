// system calls
int write(int, const void*, int);
int close(int);
int open(const char*, int);
int exec(char*);
int uptime(void);

void printf(int, const char*, ...);