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
	assert(fd2 >= 0);
	lseek(fd2, 6, SEEK_SET);
	rc = write(fd2, buffer, strlen(buffer));
	assert(rc == (strlen(buffer)));

	int fd3 = open("/tmp/file", O_WRONLY | O_APPEND);
	assert(fd3 >= 0);
	write(fd3, "!\n", 2);
	close(fd3);

	lseek(fd2, 0, SEEK_SET);
	rc = read(fd2, buffer, BUF_LEN);
	assert(rc > 0);
	printf("%d: %s", rc, buffer);
	close(fd2);

	return 0;
}
/** open
The return value of open() is a file descriptor, a small,
       nonnegative integer that is an index to an entry in the process's
       table of open file descriptors.  The file descriptor is used in
       subsequent system calls (read(2), write(2), lseek(2), fcntl(2),
       etc.)  to refer to the open file.  The file descriptor returned by
       a successful call will be the lowest-numbered file descriptor not
       currently open for the process.

A call to open() creates a new open file description, an entry in
       the system-wide table of open files.  The open file description
       records the file offset and the file status flags.

The argument flags must include one of the following access modes:
       O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file
       read-only, write-only, or read/write, respectively.

O_TRUNC: If the file already exists and is a regular file and the
					 access mode allows writing (i.e., is O_RDWR or O_WRONLY) it
					 will be truncated to length 0.
O_CREAT: If path does not exist, create it as a regular file.
O_APPEND: The file is opened in append mode.  Before each write(2),
            the file offset is positioned at the end of the file, as if
            with lseek(2).  The modification of the file offset and the
            write operation are performed as a single atomic step.
S_IRUSR:  00400 user has read permission
S_IWUSR:  00200 user has write permission
**/

/** lseek
SEEK_SET: The file offset is set to offset bytes.
SEEK_CUR: The file offset is set to its current location plus offset bytes.
SEEK_END: The file offset is set to the size of the file plus offset bytes.
**/
