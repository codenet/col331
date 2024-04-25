#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

#define SZ 1000000
#define ITER 1000

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t lock;
volatile int x[SZ] = {0};

void* inc(void* arg) {
	for(int k = 0; k < ITER; k++) {
		int i = rand() % SZ;
		int j = rand() % SZ;
		// pthread_mutex_lock(&m);
		pthread_rwlock_wrlock(&lock);
		x[i]+=1;
		x[j]-=1;
		pthread_rwlock_unlock(&lock);
		// pthread_mutex_unlock(&m);
	}
	return NULL;
}

void* sum(void* arg) {
	for(int k = 0; k < ITER; k++) {
		int s = 0;
		// pthread_mutex_lock(&m);
		pthread_rwlock_rdlock(&lock);
		for(int i = 0; i < SZ; i++) {
			s += x[i];
		}
		pthread_rwlock_unlock(&lock);
		// pthread_mutex_unlock(&m);
		if(s != 0) {
			printf("oops! sum is %d\n", s);
			exit(-1);
		}
	}
	return NULL;
}

int main() {
	int num_threads = 8;
	pthread_t p[num_threads];

	pthread_rwlock_init(&lock, NULL);
	pthread_create(&p[0], NULL, inc, NULL);
	for(int i = 1; i < num_threads; i++)
		pthread_create(&p[i], NULL, sum, NULL);

	for(int i = 0; i < num_threads; i++)
		pthread_join(p[i], NULL);
}
