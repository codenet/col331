#include<stdio.h>
#include<stdlib.h>

int main() {
	int* x = malloc(sizeof(int) * 100);
	x[100] = 0;
	printf("%d\n", x[100]);
}
