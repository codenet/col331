#include<stdio.h>
#include<stdlib.h>
int main() {
	int* x = (int*) malloc(10*sizeof(int));
	// printf("x: %p\n", x);
	free(x);

	int* y = (int*) malloc(10*sizeof(int));
	// printf("y: %p\n", y);
	free(x);

	int* z = (int*) malloc(10*sizeof(int));
	// printf("x: %p\n", x);
	for(int i=0; i < 10; i++) {
		z[i] = i;
	}
	y[8] = 3;
	for(int i=0; i < 10; i++) {
		printf("%d, %d\n", i, z[i]);
	}
}
