#include <stdio.h>
#include <unistd.h> 
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define BUF_LEN 20

int main(int argc, char *argv[]) {
	int fd = open("/tmp/fstest", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	assert(fd >= 0);
	char buffer[BUF_LEN];
	sprintf(buffer, "hello world\n");
	for(int i = 0; i < 100; i++) {
		int rc = write(fd, buffer, strlen(buffer));
		assert(rc == (strlen(buffer)));
		fsync(fd);
	}
	close(fd);
}

