// From https://stackoverflow.com/questions/47456497/multi-threading-petersons-algorithm-not-working

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "common_threads.h"

const int PRODUCER = 0,CONSUMER =1;
volatile int counter;
volatile int flag[2];
volatile int turn;

void* producer() {
	flag[PRODUCER]=1;
	turn=CONSUMER;
	while(flag[CONSUMER] && turn==CONSUMER);
	counter++;
	flag[PRODUCER]=0;
	return NULL;
}

void* consumer() {
	flag[CONSUMER]=1;
	turn=PRODUCER;
	while(flag[PRODUCER] && turn==PRODUCER);
	counter--;
	flag[CONSUMER]=0;
	return NULL;
}

int main(int argc, char *argv[])
{
	int iter =0;
	counter=0;
	while(counter==0) {
		pthread_t tid[2];

		counter=0;
		flag[0]=flag[1]=0;
		turn = 0;

		pthread_create(&tid[0],NULL,producer,NULL);
		pthread_create(&tid[1],NULL,consumer,NULL);

		pthread_join(tid[0],NULL);
		pthread_join(tid[1],NULL);

		printf("counter is %d in iteration %d\n",counter, iter++);
	}

	return 0;
}
