all: threads threads-notv

CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie 

threads: threads.c common.h common_threads.h
	gcc $(CFLAGS) -m32 -O1 -S threads.c
	gcc $(CFLAGS) -m32 -o threads threads.s -Wall -pthread

threads-notv: threads-notv.c common.h common_threads.h
	gcc $(CFLAGS) -m32 -O1 -S threads-notv.c
	gcc $(CFLAGS) -m32 -o threads-notv threads-notv.s -Wall -pthread

clean:
	rm *.s threads threads-notv
