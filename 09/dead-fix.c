#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ACCS 10
#define TXNS 100

typedef struct _account_t {
	int id;
	pthread_mutex_t lock;
	int balance;
} account_t;

typedef struct _txn_t {
	int id;
	account_t* src;
	account_t* dst;
	int amount;
	pthread_t thr;
} txn_t;

void* transfer(void* arg) {
	txn_t* t = (txn_t*) arg;
	printf("%d src: %d dst: %d\n", t->id, t->src->id, t->dst->id);

	if(t->src->id < t->dst->id) {
		pthread_mutex_lock(&t->src->lock);
		printf("%d got src: %d\n", t->id, t->src->id);
		pthread_mutex_lock(&t->dst->lock);
		printf("%d got dst: %d\n", t->id, t->dst->id);
	} else {
		pthread_mutex_lock(&t->dst->lock);
		printf("%d got dst: %d\n", t->id, t->dst->id);
		pthread_mutex_lock(&t->src->lock);
		printf("%d got src: %d\n", t->id, t->src->id);
	}

	if(t->src->balance > t->amount) {
		t->dst->balance += t->amount;
		t->src->balance -= t->amount;
	}

	pthread_mutex_unlock(&t->src->lock);
	printf("release src: %d\n", t->src->id);
	pthread_mutex_unlock(&t->dst->lock);
	printf("release dst: %d\n", t->dst->id);

	return NULL;
}

int main() {
	txn_t t[TXNS];
	account_t accs[ACCS];
	for(int i = 0; i < ACCS; i++) {
		pthread_mutex_init(&accs[i].lock, NULL);
		accs[i].balance = 1000;
		accs[i].id = i;
	}
	for(int i = 0; i < TXNS; i++) {
		int s = rand() % ACCS;
		int d = (i%ACCS);
		if (s == d)
			d = (s+1)%ACCS;

		t[i].id = i;
		t[i].src = &accs[s];
		t[i].dst = &accs[d];
		t[i].amount = 10;
		pthread_create(&t[i].thr, NULL, transfer, (void*)&t[i]);
	}
	for(int i = 0; i < TXNS; i++) {
		pthread_join(t[i].thr, NULL);
	}
}
