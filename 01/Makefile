all: cpu mem threads io

clean:
	rm -f cpu mem threads io

cpu: cpu.c common.h
	gcc -o cpu cpu.c -Wall

mem: mem.c common.h
	# setarch $(uname -m) -R ./mem 1
	gcc -o mem mem.c -Wall -Wl,-no_pie

threads: threads.c common_threads.h
	gcc -o threads threads.c -Wall -pthread

io: io.c 
	gcc -o io io.c -Wall

