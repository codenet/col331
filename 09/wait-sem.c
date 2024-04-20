#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "common.h"
#include "common_threads.h"

volatile int done = 0;
sem_t s;

void *child(void *arg) { 
	int* counter = (int*) arg;
	printf("child begin %d\n", *counter); 
	sem_post(&s);
	printf("child end %d\n", *counter); 
	return NULL; 
}

void *parent(void *arg) {
	int* counter = (int*) arg;
	pthread_t ch; 
	printf("parent begin %d\n", *counter); 
	int ret = pthread_create(&ch, NULL, child, arg); // create child 
	if(ret != 0) {
		printf("Failed to create a thread %d\n", ret);
		return NULL;
	}
	sem_wait(&s);
	printf("parent end %d\n", *counter); 
	pthread_join(ch, NULL);
	return NULL; 
}

int main(int argc, char *argv[]) { 
	pthread_t p; 
	int counter = 0;
	sem_init(&s, 0, 0);
	while(1) {
		pthread_create(&p, NULL, parent, (void*)&counter); // create parent 
		pthread_join(p, NULL);
		counter++;
	}
	return 0;
}
