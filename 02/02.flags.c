#include "common.h"
int foo(int x, int y) {
	int z = x + y;
	if(z % 2 == 0)
		return x;
	return y;
}
