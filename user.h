// system calls
int write(int, const void*, int);
int close(int);
int open(const char*, int);
int exec(char*);

// ulib.c
void printf(int, const char*, ...);