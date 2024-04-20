#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "common_threads.h"
#define SZ 2

volatile char buf[SZ];
volatile int reader = 0;
volatile int writer = 0;

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void pipe_write(char c) {
	pthread_mutex_lock(&m);
	if((writer + 1) % SZ == reader)
		pthread_cond_wait(&full, &m);
	buf[writer] = c;
	writer = (writer + 1)%SZ;
	pthread_mutex_unlock(&m);
	pthread_cond_signal(&empty);
}

char pipe_read() {
	pthread_mutex_lock(&m);
	if(reader == writer)
		pthread_cond_wait(&empty, &m);
	char c = buf[reader];
	reader = (reader + 1)%SZ;
	pthread_mutex_unlock(&m);
	pthread_cond_signal(&full);
	return c;
}

void *consumer(void *arg) { 
	while(1) {
		sleep(1);
		printf("%c\n", pipe_read() - 'a' + 'A');
	}
	return NULL; 
}

void *producer(void *arg) {
	while(1) {
		for(char c='a'; c<='z'; c++) {
			// sleep(1);
			pipe_write(c);
		}
	}
	return NULL; 
}

int main(int argc, char *argv[]) { 
	pthread_t p, c; 
	pthread_create(&p, NULL, producer, NULL);
	pthread_create(&c, NULL, consumer, NULL);
	pthread_join(p, NULL);
	pthread_join(c, NULL);
	return 0;
}
