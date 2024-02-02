all: io fsync

clean:
	rm -f io

io: io.c common.h
	gcc -o io io.c -Wall

fsync: fsync.c common.h
	gcc -o fsync fsync.c -Wall

