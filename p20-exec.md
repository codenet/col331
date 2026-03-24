## `exec` system call 

Now that we have the system call interface working, we add the `exec` system
call. `exec` allocates a new memory segment, loads the ELF file in the segment,
sets up the user stack, updates trapframe's instruction pointer to ELF entry
point, and trapframe's stack pointer to top of the user stack. Note that `usp`
is the "user stack pointer" which always contains virtual addresses.

Notice that in this branch, we separated user data from kernel stack into two
different memory segments. Because of this, we were able to `kfree` old address
space (user data is in `curproc->offset`).  If kernel stack was within the same
segment, we could not have called `kfree` as it writes `1`s to the entire
segment. It would have also written `1`s over the kernel stack, and then we
would have lost trapframe, return addresses, etc. unable to return from the
syscall. 

This implementation is wasteful: we allocate a segment of size PGSIZE but only
use KSTACKSIZE for kstack. We could improve this by making our kernel memory
allocator smarter (able to handle allocations of both KSTACKSIZE and PGSIZE).
This problem will go away when we do paging (then KSTACKSIZE=PGSIZE), so we will
not bother with this for now.

We prepare a `_init` file from `init.c` and add it to the file system (`mkfs.c` 
drops the leading _, so the file is saved as "/init").  `initcode.S` invokes the
`exec` system call to give control to `init.c` which just prints something on
the console. To print things to the console, it uses `printf` which itself makes
system calls to write each letter. The code in `printf.c` is very similar to
that in `console.c`. `console.c` was sending each letter to UART interface where
`printf.c` is making system calls.

The system calls are made available to `init.c` by `user.h` which are in-turn
defined in `usys.S`.
