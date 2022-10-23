## Free space management

Now that we were able to start an isolated process and handle interrupts while
it was running, we would like to be able to start multiple processes. We will
later make them more interesting than just running an infinite loop.

To start multiple processes, we need to map their memory segments to different
locations in memory. Previously, we hardcoded that the memory segment of our
`initproc` starts from 2MB (`STARTPROC`). Since, processes start at any point of
time, we would like to dynamically figure out where we have free space to start
a new process. Similarly, since processes may be stopping dynamically, we would
also like to mark the memory owned by the process as free so that it can be
given to the next process that starts.

> We will see processes exitting in action later when we implement ZOMBIE
processes.

The OS manages free space in `kalloc.c` to facilitate dynamic allocation and
deallocation of memory. `main.c` calls `kinit` which marks all the memory
regions after the kernel was loaded as free. To avoid *fragmentation*, xv6 only
allocates and de-allocates memory in `PGSIZE` chunks which we have set to 1MB
(equal to `PROCSIZE`). In a fragmented memory, we might have memory available,
but we are unable to satisfy allocation requests.

`kalloc.c` manages free regions as a linked list. Interestingly, the pointers of
the linked list are stored in the free pages themselves as seen in `kfree()`.
When an allocation request comes, `kalloc()` returns the first free page and 
moves remembers the new head of the free list.

With this, `main.c` is now able to start two `initproc` processes by calling
`pinit` twice. `allocproc` in `proc.c` maps process to a dynamically allocated
memory instead of using a hardcoded constant `STARTPROC`. Press `ctrl+P` after
the OS boots to see:

```
1 run    initcode
2 runble initcode
```
