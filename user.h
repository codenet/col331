// system calls
struct stat;

int write(int, const void*, int);
int close(int);
int open(const char*, int);
int exec(char*, char**);

// ulib.c
void printf(int, const char*, ...);

int fork(void);
int wait(void);
int exit(void) __attribute__((noreturn));
int getpid(void);
int kill(int);

int read(int, void*, int);
int dup(int);
int fstat(int fd, struct stat*);
int stat(const char*, struct stat*);
int chdir(const char*);
int pipe(int*);
int mknod(const char*, short, short);

char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* memmove(void*, const void*, uint);
char* strchr(const char*, char);
int strcmp(const char*, const char*);
char* strcpy(char*, const char*);

int pipe(int*);
int dup(int);
