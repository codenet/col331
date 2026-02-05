#include"common.h"
int bar() {
	return exponent(41, 3);
}

int func() {
	int x = 11;
	bar();
	return foo(x, 23);
}
