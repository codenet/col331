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
		open("./p5.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);

		// redirect standard error to standard output
		close(STDERR_FILENO); 
		dup(STDOUT_FILENO);

		printf("hello, I am child (pid:%d)\n", (int) getpid());
		// now exec "ls"...
		char *myargs[4];
		myargs[0] = strdup("ls");   // program: "ls"
		myargs[1] = strdup("p5.c"); // argument: a file that exists
		myargs[2] = strdup("p5.xyz"); // argument: a random file that does not exist
		myargs[3] = NULL;           // marks end of array
		execvp(myargs[0], myargs);  // runs ls
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
