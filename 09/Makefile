CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie -O2

all: threads-safe peterson-breaks peterson-fence atomic wait mypipe alloc semlock wait-sem sempipe sem-mpmc dine-dead dine rw-ctr rw-using-sems sems-using-lock-cv dead dead-fix

clean:
	rm threads-safe peterson-breaks peterson-fence atomic wait mypipe alloc semlock wait-sem sempipe sem-mpmc dine-dead dine rw-ctr rw-using-sems sems-using-lock-cv dead dead-fix

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

rw-ctr: rw-ctr.c common.h common_threads.h
	gcc $(CFLAGS) -o rw-ctr rw-ctr.c -Wall -pthread

rw-using-sems: rw-using-sems.c common.h common_threads.h
	gcc $(CFLAGS) -o rw-using-sems rw-using-sems.c -Wall -pthread

sems-using-lock-cv: sems-using-lock-cv.c common.h common_threads.h
	gcc $(CFLAGS) -o sems-using-lock-cv sems-using-lock-cv.c -Wall -pthread

dine-dead: dine-dead.c common.h common_threads.h
	gcc $(CFLAGS) -o dine-dead dine-dead.c -Wall -pthread

dine: dine.c common.h common_threads.h
	gcc $(CFLAGS) -o dine dine.c -Wall -pthread

dead: dead.c common.h common_threads.h
	gcc $(CFLAGS) -o dead dead.c -Wall -pthread

dead-fix: dead-fix.c common.h common_threads.h
	gcc $(CFLAGS) -o dead-fix dead-fix.c -Wall -pthread

