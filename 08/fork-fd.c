#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	char b[] = "hello world\n";
	int fd = open("fd.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write(fd, b, strlen(b));
	int rc = fork();
	if (rc < 0) {
		// fork failed; exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		// child (new process)
		char c[] = "hi, I am the child\n";
		write(fd, c, strlen(c));
	} else {
		// parent goes down this path (original process)
		char c[] = "hi, I am the parent\n";
		write(fd, c, strlen(c));
	}
	close(fd);
	return 0;
}
