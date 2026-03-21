// system calls
int write(int, const void*, int);
int close(int);
int open(const char*, int);
int exec(char*);
int uptime(void);
int sleep(int);
int mknod(const char*, short, short);

void printf(int, const char*, ...);