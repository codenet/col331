#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0; 
int loops;
pthread_mutex_t lock;

void *worker(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		pthread_mutex_lock(&lock);
		counter++;
		pthread_mutex_unlock(&lock);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) { 
		fprintf(stderr, "usage: threads <loops>\n"); 
		exit(1); 
	} 
	int rc = pthread_mutex_init(&lock, NULL);
	if (rc != 0) { 
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
	pthread_mutex_destroy(&lock);
	return 0;
}

