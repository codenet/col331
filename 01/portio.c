#include <stdio.h>
#include <sys/io.h>

#define PORT 0x378 /* Parallel port base address */

int main() {
	if (ioperm(PORT, 1, 1)) {
		perror("ioperm");
		return 1;
	}

	unsigned char data = inb(PORT);
	printf("Read: %u\n", data);

	outb(255, PORT);
	printf("Wrote: 255\n");

	return 0;
}