all: 02.main.dump 02.dump

CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie 

# compile
02.flags.s: 02.flags.c
	gcc $(CFLAGS) -S -O1 02.flags.c

02.eip.s: 02.eip.c
	gcc $(CFLAGS) -S -O1 02.eip.c

02.main.s: 02.main.c
	gcc $(CFLAGS) -S -O1 02.main.c

02.s: 02.c
	gcc $(CFLAGS) -S 02.c

02.func.s: 02.func.c
# We don't use -O1 here since O1 optimizes away the function call
	gcc $(CFLAGS) -S 02.func.c

# assemble
02.flags.o: 02.flags.s
	gcc $(CFLAGS) -c 02.flags.s -o 02.flags.o

02.eip.o: 02.eip.s
	gcc $(CFLAGS) -c 02.eip.s -o 02.eip.o

02.main.o: 02.main.s
	gcc $(CFLAGS) -c 02.main.s -o 02.main.o

02.func.o: 02.func.s
	gcc $(CFLAGS) -c 02.func.s -o 02.func.o

02.o: 02.s
	gcc $(CFLAGS) -c 02.s -o 02.o

# link
02.main: 02.flags.o 02.eip.o 02.main.o 02.func.o
	gcc $(CFLAGS) 02.flags.o 02.eip.o 02.main.o 02.func.o -o 02.main

# disassemble
02.main.dump: 02.main
	objdump -d 02.main > 02.main.dump

02.dump: 02.o
	objdump -d 02.o > 02.dump

clean:
	rm *.o *.s *.dump
