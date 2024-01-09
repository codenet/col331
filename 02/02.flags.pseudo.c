int foo(int x, int y) {
	int eax, edx;
	bool zf;
	eax = x;
	edx = eax;
	edx += y;
	edx = (edx & 1); // z = z % 2
	zf = (edx == 0); // zf = (z%2 == 0)

	if (!zf)
		eax = y;
	return eax;
}
