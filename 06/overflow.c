// Disable ASLR to see overflow
// setarch `uname -m` -R /bin/bash
#include<stdio.h>
#include<stdlib.h>

int main() {
	int* x = (int*) malloc(sizeof(int) * 100);
	int* y = (int*) malloc(sizeof(int));
	// printf("%p %p\n", x, y);
	*y = 23;
	x[104] = 0;
	printf("y: %d\n", *y);
}
