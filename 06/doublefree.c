#include<stdio.h>
#include<stdlib.h>
int main() {
	int* x = (int*) malloc(100*sizeof(int));
	printf("x: %p\n", x);
	x[23] = 3;
	free(x);

	int* y = (int*) malloc(100*sizeof(int));
	printf("y: %p\n", y);
	free(x);
	y[23] = 3;
}
