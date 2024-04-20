#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "common_threads.h"
#define SZ 1000

volatile int bytes_left = 0;

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void my_allocate(int size) {
	pthread_mutex_lock(&m);
	while(bytes_left < size) {
		printf("Wanted %d bytes. Only %d available\n", size, bytes_left);
		pthread_cond_wait(&c, &m);
	}
	// *ptr = 
	bytes_left -= size;
	pthread_mutex_unlock(&m);
	// return ptr;
}

void my_free(int size) {
	printf("Freeing %d bytes\n", size);
	pthread_mutex_lock(&m);
	bytes_left += size;
	// pthread_cond_signal(&c);
	pthread_cond_broadcast(&c);
	pthread_mutex_unlock(&m);
}

void *alloc(void* arg) {
	int* s = (int*) arg;
	my_allocate(*s);
	printf("Thread got %d bytes\n", *s);
	sleep(5);
	my_free(*s);
	return NULL; 
}

int main(int argc, char *argv[]) { 
	int num_threads = 10;
	pthread_t t[num_threads]; 
	int szs[num_threads];

	for(int i = 0; i < num_threads; i++) {
		szs[i] = (num_threads-i) * 100;
		pthread_create(&t[i], NULL, alloc, (void*) &szs[i]);
	}

	sleep(1);
	my_free(900);

	for(int i = 0; i < num_threads; i++) {
		pthread_join(t[i], NULL);
	}
	return 0;
}
