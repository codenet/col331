#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int s = atoi(argv[1]);
    printf("location of code : %p\n", main);
    printf("location of heap : %p\n", malloc(s));
    int x = 3;
    printf("location of stack: %p\n", &x);
    return 0;
}
