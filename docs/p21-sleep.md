# Sleep and wakeup

This branch adds the facility to let processes voluntarily give up the CPU.  For
example, `init.c` sleeps for 100 timer ticks within an infinite loop.

To realize this, we add a new process state called `SLEEPING` so that scheduler
can skip scheduling `SLEEPING` processes. In xv6, processes sleep on "channels".
Processes can call `sleep(chan)` to sleep on a channel `chan`.  At some later
stage, `wakeup(chan)` can wakeup all processes sleeping on `chan`. Waking up a
process just means setting its state to `RUNNABLE`.  Whenever, the scheduler is
invoked next, these processes can now get scheduled.

For example, the sleep system call implementation (`sys_sleep`) calls
`sleep(&ticks)` i.e., it sleeps on the `ticks` variable managed by the timer
interrupt handler in `trap.c`. Whenever a timer interrupt is triggered, the
timer interrupt handler additionally calls `wakeup(&ticks)` to wake up all the
processes who had called the `sleep` system call. After waking up, the system 
call handler `sys_sleep` re-checks if enough ticks have elapsed. Otherwise, it
sleeps on `&ticks` again.

Notice that the implementation of `sys_sleep` is exceptionally unoptimized.
Firstly, if a process sleeps for 1000s of ticks, it is woken up on every tick!
It is marked `RUNNABLE`, it gets scheduled sometime later in the future, where
it gets put to sleep again. Secondly, *every* tick wakes up *every* sleeping
process. This is often called the *thundering herd* problem. Delta lists,
pioneered by XINU, is a cute data structure to avoid both these problems. 