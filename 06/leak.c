#include<stdio.h>
#include<stdlib.h>
int main(int argc, char** argv) {
	int s = atoi(argv[1]);
	for(int i = 0; i < 10000; i++) {
		int* x = (int*) malloc(1000000*sizeof(int));
		for(int j = 0; j < 1000000; j+=4096)
			x[j] = j;
		printf("%d\n", x[s]);
		// free(x);
	}
}
