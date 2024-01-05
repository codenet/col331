#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0; 
// volatile used to indicate to the compiler that a variable's value may change unexpectedly
// volatile is needed to make sure that compiler doesn't do any optimisations such as storing the value of counter
// in any registers to prevent race conditions
// But still, we are trying to access the value of the counter in the critical section without a lock 
// so a race condition can still occur due to our own fault of writing the incorrect programme.
int loops;

void *worker(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		counter++;
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) { 
		fprintf(stderr, "usage: threads <loops>\n"); 
		exit(1); 
	} 
	loops = atoi(argv[1]);
	pthread_t p1, p2;
	printf("Initial value : %d\n", counter);
	Pthread_create(&p1, NULL, worker, NULL); 
	Pthread_create(&p2, NULL, worker, NULL);
	Pthread_join(p1, NULL);
	Pthread_join(p2, NULL);
	printf("Final value   : %d\n", counter);
	return 0;
}

