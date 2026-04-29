#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
	printf("hello world (pid:%d)\n", (int) getpid());
	int rc = fork();
	if (rc < 0) {
		// fork failed; exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		// child: redirect standard output to a file
		close(STDOUT_FILENO); 
		int fd = open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);

		printf("hello, I am child (pid:%d) with fd=%d\n", (int) getpid(), fd);
		// now exec "wc"...
		char *myargs[3];
		myargs[0] = strdup("wc");   // program: "wc" (word count)
		myargs[1] = strdup("p4.c"); // argument: file to count
		myargs[2] = NULL;           // marks end of array
		execvp(myargs[0], myargs);  // runs word count
		printf("this shouldn't print out");
	} else {
		// parent goes down this path (original process)
		int wc = wait(NULL);
		printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
				rc, wc, (int) getpid());
		assert(wc >= 0);
	}
	return 0;
}
