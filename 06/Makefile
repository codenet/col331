all: va null leak uninitread overflow useafterfree invalidfree doublefree nicedemo

clean:
	rm -f va null leak uninitread overflow useafterfree invalidfree doublefree nicedemo

va: va.c
	gcc -o va va.c -Wall -no-pie

null: null.c
	gcc -g -o null null.c

leak: leak.c
	gcc -g -m32 -O1 -o leak leak.c

overflow: overflow.c
	gcc -g -o overflow overflow.c

useafterfree: useafterfree.c
	gcc -g -o useafterfree useafterfree.c

invalidfree: invalidfree.c
	gcc -g -o invalidfree invalidfree.c

doublefree: doublefree.c
	gcc -g -o doublefree doublefree.c

uninitread: uninitread.c
	gcc -g -o uninitread uninitread.c

nicedemo: nicedemo.c
	gcc -g -o nicedemo nicedemo.c
