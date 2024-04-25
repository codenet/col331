#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>

#define SZ 1000000
#define ITER 1000

typedef struct _rwlock_t {
	sem_t lock;
	sem_t writelock;
	int readers;
} rwlock_t;

void rwlock_init(rwlock_t *rw) {
	rw->readers = 0;
	sem_init(&rw->lock, 0, 1);
	sem_init(&rw->writelock, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
	sem_wait(&rw->lock);
	rw->readers++;
	if(rw->readers == 1)	// first reader gets write lock
		sem_wait(&rw->writelock);
	sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw) {
	sem_wait(&rw->lock);
	rw->readers--;
	if(rw->readers == 0)	// last reader releases write lock
		sem_post(&rw->writelock);
	sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
	sem_wait(&rw->writelock);
}

void rwlock_release_writelock(rwlock_t *rw) {
	sem_post(&rw->writelock);
}

rwlock_t lock;
volatile int x[SZ] = {0};

void* inc(void* arg) {
	for(int k = 0; k < ITER; k++) {
		int i = rand() % SZ;
		int j = rand() % SZ;
		rwlock_acquire_writelock(&lock);
		x[i]+=1;
		x[j]-=1;
		rwlock_release_writelock(&lock);
	}
	return NULL;
}

void* sum(void* arg) {
	for(int k = 0; k < ITER; k++) {
		int s = 0;
		rwlock_acquire_readlock(&lock);
		for(int i = 0; i < SZ; i++) {
			s += x[i];
		}
		rwlock_release_readlock(&lock);
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

	rwlock_init(&lock);
	pthread_create(&p[0], NULL, inc, NULL);
	for(int i = 1; i < num_threads; i++)
		pthread_create(&p[i], NULL, sum, NULL);

	for(int i = 0; i < num_threads; i++)
		pthread_join(p[i], NULL);
}
