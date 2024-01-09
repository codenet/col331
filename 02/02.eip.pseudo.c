int exponent(int x, int y) {
	int eax, ecx, edx;

	ecx = x;
	eax = y;
	int v = (eax & eax);
	bool sf = v & (1<<31);
	bool zf = (v==0);
	
	if(sf || zf) 	// if(y<=0)
		goto L4;

	edx = ecx;	// z = x
L3:
	edx *= ecx;	// z *= x
	eax--;		// (y--)
	zf = (eax==0);
	if(!zf)		// if(y != 0)
		goto L3;
L1:
	eax = edx;
	return eax;
L4:
	edx = ecx;
	goto L1;
}
