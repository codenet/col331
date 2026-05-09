#include<stdio.h>
#define NUM 10000000
#define FENCE 1
volatile int flag=0, counter = 0;
int producer() {
  for(int i=0; i < NUM; i++) {
		flag=1;
		if(FENCE) asm volatile ("mfence");
		counter++;
		if(FENCE) asm volatile ("mfence");
		flag=0;
		if(FENCE) asm volatile ("mfence");
	}
	return counter;
}

int main(int argc, char *argv[]) {
	printf("%d", producer());
}
