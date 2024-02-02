#include <stdio.h>
#include <unistd.h> 
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define BUF_LEN 20

int main(int argc, char *argv[]) {
	int fd = open("/tmp/file", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	assert(fd >= 0);
	char buffer[BUF_LEN];
	sprintf(buffer, "hello world\n");
	int rc = write(fd, buffer, strlen(buffer));
	assert(rc == (strlen(buffer)));
	close(fd);

	memset(buffer, 0, BUF_LEN);
	sprintf(buffer, "COL331");

	int fd2 = open("/tmp/file", O_RDWR);
	lseek(fd, 6, SEEK_SET);
	rc = write(fd, buffer, strlen(buffer));
	assert(rc == (strlen(buffer)));

	int fd3 = open("/tmp/file", O_WRONLY | O_APPEND);
	write(fd3, "!\n", 2);
	close(fd3);

	lseek(fd2, 0, SEEK_SET);
	rc = read(fd2, buffer, BUF_LEN);
	assert(rc > 0);
	printf("%d: %s", rc, buffer);
	close(fd2);

	return 0;
}

