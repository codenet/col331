int exponent(int x, int y) {
	int z = x;
	while(y > 0) {
		z = z * x;
		y --;
	}
	return z;
}
