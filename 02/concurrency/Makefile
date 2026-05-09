all: threads threads-notv threads-notv-O3

CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie 

threads: threads.c common_threads.h
	gcc $(CFLAGS) -m32 -O1 -S threads.c
	gcc $(CFLAGS) -m32 -o threads threads.s -Wall -pthread

threads-notv: threads-notv.c common_threads.h
	gcc $(CFLAGS) -m32 -O1 -S threads-notv.c
	gcc $(CFLAGS) -m32 -o threads-notv threads-notv.s -Wall -pthread

threads-notv-O3: threads-notv.c common_threads.h
	gcc $(CFLAGS) -m32 -O3 -S threads-notv.c -o threads-notv.O3.s
	gcc $(CFLAGS) -m32 -o threads-notv-O3 threads-notv.O3.s -Wall -pthread

clean:
	rm -f *.s threads threads-notv threads-notv-O3
