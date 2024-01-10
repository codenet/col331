ecx = loops;
if (ecx > 0) {
	edx = 0;

	while(True) {
		eax = counter;
		eax ++;
		counter = eax;
		edx ++;
		if(edx == ecx) break;
	}
}
eax = 0;
return;
