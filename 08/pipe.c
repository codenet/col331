#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	int p[2];
	if(pipe(p) < 0) return -1;
	int rc = fork();
	if (rc < 0) {
		// fork failed; exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		// child: redirect standard output to a file
		close(STDOUT_FILENO);
		dup(p[1]);
		close(p[0]);
		close(p[1]);

		// now exec "wc"...
		char *myargs[3];
		myargs[0] = strdup("echo");
		myargs[1] = strdup("i will get printed in capital letters!!");
		myargs[2] = NULL;           // marks end of array
		execvp(myargs[0], myargs);  // runs word count
	} else {
		// parent goes down this path (original process)
		close(p[1]);
		close(STDIN_FILENO);
		dup(p[0]);
		char* c = (char*) calloc(1, sizeof(char));
		while (c[0] != EOF) { 
			int r = read(STDIN_FILENO, c, sizeof(char));
			if(r < 1) return 0;
			if(c[0]>='a' && c[0] <= 'z')
				c[0] = c[0] - 'a' + 'A';
			putchar(c[0]); 
		} 
	}
	return 0;
}
