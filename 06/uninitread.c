#include<stdio.h>
#include<stdlib.h>

int main() {
	int *x = malloc(sizeof(int));
	printf("%d\n", *x);
	free(x);
}
