#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
volatile int done = 0;

void *child(void *arg) { 
	int* counter = (int*) arg;
	printf("child begin %d\n", *counter); 
	pthread_mutex_lock(&m);
	done = 1; // 4 // 2
	pthread_cond_signal(&c); // 5 // 3
	pthread_mutex_unlock(&m);
	printf("child end %d\n", *counter); 
	return NULL; 
}

void *parent(void *arg) {
	int* counter = (int*) arg;
	pthread_t ch; 
	printf("parent begin %d\n", *counter); 
	int ret = pthread_create(&ch, NULL, child, arg); // 1
	if(ret != 0) {
		printf("Failed to create a thread %d\n", ret);
		return NULL;
	}
	pthread_mutex_lock(&m);
	if(done == 0) // 2 // 4
		pthread_cond_wait(&c, &m); // 3
	pthread_mutex_unlock(&m);
	printf("parent end %d\n", *counter); 
	pthread_join(ch, NULL);
	return NULL; 
}

int main(int argc, char *argv[]) { 
	pthread_t p; 
	int counter = 0;
	while(1) {
		done = 0;
		pthread_create(&p, NULL, parent, (void*) &counter); // create parent 
		pthread_join(p, NULL);
		counter++;
	}
	return 0;
}
