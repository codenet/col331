#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	FILE* f = fopen("fd2.txt", "w");
	fprintf(f, "hello world\n");
	fflush(f);
	int rc = fork();
	if (rc < 0) {
		// fork failed; exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		// child (new process)
		fprintf(f, "hi, I am the child\n");
	} else {
		// parent goes down this path (original process)
		fprintf(f, "hi, I am the parent\n");
	}
	fclose(f);
	return 0;
}
