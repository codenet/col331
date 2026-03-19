# Locks

With multiple processes running, we have *concurrency* in the system, i.e.,
multiple instruction sequences can be interleaved.  There is no *parallelism*
yet, since we only have a single CPU, i.e., only one instruction sequence can
run at a *given* time instant.

With a different design, we could have had concurrency within the OS, as early
as the [p5-input branch](https://github.com/codenet/col331/blob/p5-input/main.c)
i.e., as soon as we started handling interrupts.  For example, instead of just
waiting for interrupts (`wfi`) in an infinite loop, `main` could be running some
computation. This computation could get interrupted by the timer.  While we were
handling the timer interrupt, we could again get interrupted by the keyboard. If
we let the above situation happen, we have concurrency within the OS:
computation within `main`, handling of timer interrupt, and handling of
keyboard interrupt, are running in an interleaved manner.

However, this was not possible because (a) `main` was just waiting for
interrupts, and (b) interrupts on x86 clear the interrupt flag, i.e., disable
further interrupts. Our interrupt handlers do not re-enable interrupts by
calling `sti`. Therefore, timer interrupt handling cannot be interrupted by
keyboard interrupt handling. Interrupts get re-enabled when `iret` recovers
`eflags` in `trapret` defined in `trapasm.S` (recall that interrupt flag is a
bit inside the `eflags` register).

However when processes make system calls, the hardware does not disable
interrupts. Recall that `tvinit` marked system call's IDT entry as a "trap
gate". Because of this, we now have concurrency in the system which can lead to
***data races***.

Consider a user process P1 that makes a `read` system call which tries to read a
disk block into an empty buffer from the buffer cache:
`sys_read` -> `fileread` -> `iread` -> `bread` -> `iderw`.

Inside `iderw`, the system call has to enqueue a buffer, say `b1` into the
`idequeue`. It traverses the buffers in `idequeue` to reach the last buffer, say
`l`, whose `qnext` is null.

```c
for(pp=&idequeue; *pp; pp=&(*pp)->qnext);
// context switch
*pp = b;
```

However, since interrupts are enabled, just before it could execute `*pp=b`,
i.e., append `b1` to the queue by changing `l->qnext`, the OS gets a timer
interrupt which schedules process P2. Process P2 also makes the `read` system call
and changes `l->qnext` to its own buffer `b2`. 

Now, when P1 reruns, it continues from the line where it was context-switched
out. It overwrites `l->qnext` from `b2` to `b1`. This situation is called a
*data race*: P1 and P2 were *racing* to write to `l->qnext`. 

Races are bad when there are losers. In the above situation, the process P2 will
wait forever for its buffer `b2` to be read since it is not even in the
`idequeue`!  Even though P2 "won the race" by writing to `l->qnext` first, it
lost its buffer :-)

To avoid such unfortunate interleavings, we surround such *critical sections*
with `acquire` and `release` of locks. Only one instruction sequence should be
able to acquire a lock at a time. With these locks, we say that execution of
critical sections become *mutually exclusive*. Such locks are also sometimes
called mutual exclusion locks or mutexes. Since there is no parallelism,
`acquire` can disable interrupts, and `release` can enable it.

This branch adds lock acquires and releases around all the kernel data
structures which may be modified from system calls. To properly enable-disable
interrupts, we need to do some additional bookkeeping. 

Firstly, consider `cons.lock` in `console.c`.  `cprintf` can be called from
system call handlers like in `exec.c` or from interrupt handlers in `trap.c`.
Therefore, when we release `cons.lock` in `cprintf`, we do not want to
unconditionally enable interrupts, as we do not want to take interrupts while
handling interrupts. Therefore, `acquire` remembers whether interrupts were
enabled in an `intena` flag; `release` calls `sti` only if `intena` was set.

Secondly, we may try to acquire a lock when another one was already held. For
example, `procdump` in `proc.c` acquires a `ptable.lock` and then calls
`cprintf` which itself acquires `cons.lock`. When `cprintf` releases
`cons.lock`, we should not immediately enable interrupts as it essentially also
releases `ptable.lock`. Therefore, `spinlock.c` maintains a counter `ncli`; the
interrupts are enabled only when `ncli` becomes zero.
