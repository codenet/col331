CFLAGS=-fcf-protection=none -fno-asynchronous-unwind-tables -m32 -fno-pie -no-pie -O2

all: threads-safe peterson-breaks peterson-fence

clean:
	rm threads-safe peterson-breaks peterson-fence

threads-safe: threads-safe.c common.h common_threads.h
	gcc $(CFLAGS) -o threads-safe threads-safe.c -Wall -pthread

peterson-breaks: peterson-breaks.c common.h common_threads.h
	gcc $(CFLAGS) -S peterson-breaks.c -Wall -pthread
	gcc $(CFLAGS) -o peterson-breaks peterson-breaks.c -Wall -pthread

peterson-fence: peterson-fence.c common.h common_threads.h
	gcc $(CFLAGS) -S peterson-fence.c -Wall -pthread
	gcc $(CFLAGS) -o peterson-fence peterson-fence.c -Wall -pthread
