#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "common.h"
#include "common_threads.h"
#define SZ 10

volatile char buf[SZ];
volatile int reader = 0;
volatile int writer = 0;

sem_t write_slots;
sem_t read_slots;

void pipe_write(char c) {
	sem_wait(&write_slots);
	buf[writer] = c;
	writer = (writer + 1)%SZ;
	sem_post(&read_slots);
}

char pipe_read() {
	sem_wait(&read_slots);
	char c = buf[reader];
	reader = (reader + 1)%SZ;
	sem_post(&write_slots);
	return c;
}

void *consumer(void *arg) { 
	while(1) {
		// sleep(1);
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
	sem_init(&write_slots, 0, SZ);
	sem_init(&read_slots, 0, 0);
	pthread_t p, c; 
	pthread_create(&p, NULL, producer, NULL);
	pthread_create(&c, NULL, consumer, NULL);
	pthread_join(p, NULL);
	pthread_join(c, NULL);
	return 0;
}
