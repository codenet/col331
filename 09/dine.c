#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define PHIL 5

sem_t forks[PHIL];
int left(int p) { return p; } 
int right(int p) { return (p + 1) % PHIL; }

void* dine(void* arg) {
	int* id = (int*) arg;
	while(1) {
		printf("id: %d\n", *id);
		if(*id == 0) {
			sem_wait(&forks[left(*id)]);
			printf("id: %d got left\n", *id);
			sleep(*id);
			sem_wait(&forks[right(*id)]);
			printf("id: %d got right\n", *id);
		} else {
			sem_wait(&forks[right(*id)]);
			printf("id: %d got right\n", *id);
			sleep(*id);
			sem_wait(&forks[left(*id)]);
			printf("id: %d got left\n", *id);
		}
		sem_post(&forks[left(*id)]);
		sem_post(&forks[right(*id)]);
		printf("id: %d released both\n", *id);
	}
	return NULL;
}

int main() {
	pthread_t phils[PHIL];
	int ids[PHIL];
	for(int i = 0; i < PHIL; i++) {
		ids[i] = i;
		sem_init(&forks[i], 0, 1);
		pthread_create(&phils[i], NULL, dine, (void*)&ids[i]);
	}
	for(int i = 0; i < PHIL; i++)
		pthread_join(phils[i], NULL);
}
