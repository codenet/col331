CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie -O2

all: threads-safe peterson-breaks peterson-fence atomic wait mypipe alloc semlock wait-sem sempipe sem-mpmc

clean:
	rm threads-safe peterson-breaks peterson-fence atomic wait mypipe alloc semlock wait-sem sempipe sem-mpmc

threads-safe: threads-safe.c common.h common_threads.h
	gcc $(CFLAGS) -o threads-safe threads-safe.c -Wall -pthread

peterson-breaks: peterson-breaks.c common.h common_threads.h
	gcc $(CFLAGS) -S peterson-breaks.c -Wall -pthread
	gcc $(CFLAGS) -o peterson-breaks peterson-breaks.c -Wall -pthread

peterson-fence: peterson-fence.c common.h common_threads.h
	gcc $(CFLAGS) -S peterson-fence.c -Wall -pthread
	gcc $(CFLAGS) -o peterson-fence peterson-fence.c -Wall -pthread

atomic: atomic.c common.h common_threads.h
	gcc $(CFLAGS) -S atomic.c -Wall -pthread
	gcc $(CFLAGS) -o atomic atomic.c -Wall -pthread

wait: wait.c common.h common_threads.h
	gcc $(CFLAGS) -o wait wait.c -Wall -pthread

mypipe: mypipe.c common.h common_threads.h
	gcc $(CFLAGS) -o mypipe mypipe.c -Wall -pthread

alloc: alloc.c common.h common_threads.h
	gcc $(CFLAGS) -o alloc alloc.c -Wall -pthread

semlock: semlock.c common.h common_threads.h
	gcc $(CFLAGS) -o semlock semlock.c -Wall -pthread

wait-sem: wait-sem.c common.h common_threads.h
	gcc $(CFLAGS) -o wait-sem wait-sem.c -Wall -pthread

sempipe: sempipe.c common.h common_threads.h
	gcc $(CFLAGS) -o sempipe sempipe.c -Wall -pthread

sem-mpmc: sem-mpmc.c common.h common_threads.h
	gcc $(CFLAGS) -o sem-mpmc sem-mpmc.c -Wall -pthread
