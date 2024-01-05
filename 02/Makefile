all: 02.flags.s 02.eip.s

02.flags.s: 02.flags.c
	gcc -fno-asynchronous-unwind-tables -m32 -S -O1 02.flags.c

02.eip.s: 02.eip.c
	gcc -fno-asynchronous-unwind-tables -m32 -S -O1 02.eip.c

clean:
	rm *.o *.s
