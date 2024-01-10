#include"common.h"
int bar() {
	return exponent(41, 3);
}

int func() {
	int x = 11;
	bar();
	foo(x, 23);
}
