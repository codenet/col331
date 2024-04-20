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
sem_t read_lock;
sem_t write_lock;

void pipe_write(char c) {
	sem_wait(&write_slots);
	sem_wait(&write_lock);
	buf[writer] = c;
	writer = (writer + 1)%SZ;
	sem_post(&write_lock);
	sem_post(&read_slots);
}

char pipe_read() {
	sem_wait(&read_slots);
	sem_wait(&read_lock);
	char c = buf[reader];
	reader = (reader + 1)%SZ;
	sem_post(&read_lock);
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
	sem_init(&write_lock, 0, 1);
	sem_init(&read_lock, 0, 1);

	pthread_t p1, p2, c1, c2; 
	pthread_create(&p1, NULL, producer, NULL);
	pthread_create(&p2, NULL, producer, NULL);
	pthread_create(&c1, NULL, consumer, NULL);
	pthread_create(&c2, NULL, consumer, NULL);
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(c1, NULL);
	pthread_join(c2, NULL);
	return 0;
}
