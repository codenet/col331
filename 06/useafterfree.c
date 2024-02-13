#include<stdio.h>
#include<stdlib.h>
int main() {
	int* x = (int*) malloc(10*sizeof(int));
	// free(x);
	int* y = (int*) malloc(10*sizeof(int));
	for(int i = 0; i < 10; i++)
		y[i] = i;
	x[8] = 3;
	for(int i = 0; i < 10; i++)
		printf("y[%d]: %d\n", i, y[i]);
}
