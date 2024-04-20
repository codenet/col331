#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "common.h"
#include "common_threads.h"

volatile int counter = 0; 
int loops;
sem_t s;

void *worker(void *arg) {
	int i;
	for (i = 0; i < loops; i++) {
		sem_wait(&s);
		counter++;
		sem_post(&s);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) { 
		fprintf(stderr, "usage: semlock <loops>\n"); 
		exit(1); 
	} 
	int rc = sem_init(&s, 0, 1);
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
	sem_destroy(&s);
	return 0;
}

