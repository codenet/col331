#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int main(int argc, char *argv[]) {
	if (argc != 2) { 
		fprintf(stderr, "usage: mem <value>\n"); 
		exit(1); 
	} 
	int *p; 
	p = malloc(sizeof(int));
	assert(p != NULL);
	printf("addr pointed to by p: %p\n", p);
	*p = atoi(argv[1]); // assign value to addr stored in p
	while (1) {
		Spin(1);
		*p = *p + 1;
		printf("value of p: %d\n", *p);
	}
}

