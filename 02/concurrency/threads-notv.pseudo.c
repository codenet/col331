edx = loops;
if(edx > 0) {
	ecx = counter;
	eax = 0;

	while(True) {
		eax++;
		if(edx == eax) break;
	}
	edx += ecx;
	counter = edx;
}

eax = 0;
return;
