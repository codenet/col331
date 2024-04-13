#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0; 
volatile atomic_int acounter = 0; 
int loops;

void *worker(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		counter++;
		acounter++;
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) { 
		fprintf(stderr, "usage: atomic <loops>\n"); 
		exit(1); 
	} 
	loops = atoi(argv[1]);
	pthread_t p1, p2;
	Pthread_create(&p1, NULL, worker, NULL); 
	Pthread_create(&p2, NULL, worker, NULL);
	Pthread_join(p1, NULL);
	Pthread_join(p2, NULL);
	printf("non-atomic counter: %d\n    atomic counter: %d\n", counter, acounter);
	return 0;
}

