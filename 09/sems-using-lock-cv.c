#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "common_threads.h"

typedef struct _zem_t {
	int value;
	pthread_cond_t cond;
	pthread_mutex_t lock;
} zem_t;

void zem_init(zem_t *s, int value) {
	s->value = value;
	pthread_cond_init(&s->cond, NULL);
	pthread_mutex_init(&s->lock, NULL);
}

void zem_wait(zem_t *s) {
	pthread_mutex_lock(&s->lock);
	while(s->value <= 0)
		pthread_cond_wait(&s->cond, &s->lock);
	pthread_mutex_unlock(&s->lock);
}

void zem_post(zem_t *s) {
	pthread_mutex_lock(&s->lock);
	s->value++;
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->lock);

}

zem_t s;

void *child(void *arg) { 
	int* counter = (int*) arg;
	printf("child begin %d\n", *counter); 
	zem_post(&s);
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
	zem_wait(&s);
	printf("parent end %d\n", *counter); 
	pthread_join(ch, NULL);
	return NULL; 
}

int main(int argc, char *argv[]) { 
	pthread_t p; 
	int counter = 0;
	zem_init(&s, 0);
	while(1) {
		pthread_create(&p, NULL, parent, (void*)&counter); // create parent 
		pthread_join(p, NULL);
		counter++;
	}
	return 0;
}
