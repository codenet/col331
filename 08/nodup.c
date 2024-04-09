#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	char b[] = "hello world\n";
	int fd = open("nodup.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write(fd, b, strlen(b));

	int fd2 = open("nodup.txt", O_WRONLY, S_IRUSR | S_IWUSR);
	char c[] = "I am writing to fd2\n";
	write(fd2, c, strlen(c));

	char d[] = "I am writing to fd\n";
	write(fd, d, strlen(d));
	close(fd);

	char e[] = "I can write to fd2 after fd is closed\n";
	write(fd2, e, strlen(e));
	close(fd2);

	return 0;
}
