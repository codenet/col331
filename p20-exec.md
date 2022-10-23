## `exec` system call 

Now that we have the system call interface working, we add the `exec` system
call. `exec` overrides the memory contents of the process with memory contents
of an ELF file. 

We prepare a `_init` file from `init.c` and add it to the file system (`mkfs.c` 
drops the leading _, so the file is saved as "/init").  `initcode.S` invokes the
`exec` system call to give control to `init.c` which just prints something on
the console. To print things to the console, it uses `printf` which itself makes
system calls to write each letter. The code in `printf.c` is very similar to
that in `console.c`. `console.c` was sending each letter to UART interface where
`printf.c` is making system calls.

The systems calls are made available to `init.c` by `user.h` which are in-turn
defined in `usys.S`.