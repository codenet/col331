#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SZ 100000000

int
main(int argc, char *argv[])
{
    printf("hello world (pid:%d)\n", (int) getpid());
    int* a = (int*)calloc(SZ, sizeof(int));
    int sum = 23;
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
	for(int i = 0; i < SZ; i++)
		a[i]=1;
	for(int i = 0; i < SZ; i++)
		sum += a[i];
        printf("child found sum=%d\n", sum);
    } else {
        // parent goes down this path (original process)
	for(int i = 0; i < SZ; i++)
		a[i]=2;
	for(int i = 0; i < SZ; i++)
		sum += a[i];
        printf("parent found sum=%d\n", sum);
    }
    return 0;
}
