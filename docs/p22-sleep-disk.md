# Sleep on disk accesses

Building on top of sleep and wakeup introduced in the last part, this part adds
support to give up the CPU during slow disk reads/writes. In `ide.c`, when a
process enqueues a request to read/write a disk block into a buffer `b`, it
previously wasted CPU cycles in `iderw(b)` just waiting for `b` to be
read/written. Now, we just sleep on the block `b` and get woken up from the
disk interrupt handler `ideintr`.

Similarly, when starting an FS operation `begin_op` in `log.c`, if the log was
full, the OS used to panic. Now instead, we sleep on the log and wait for it to
become free.  The log becomes free after commit, where it wakes up all the
processes sleeping on the log.

With all this sleeping, we want to be careful that the CPU scheduler itself does
not sleep, i.e., it should not be the case that there is no one left to run the
scheduler. Therefore, we move initialization of the file system `iinit` and of 
the log `initlog` to starting of the first process in `proc.c`. This is because
initializing the file system reads the super block into the buffer cache, which
may sleep in `ide.c`. Initializing the log additionally reads the log header
block and optionally recovers a pending transaction which may again sleep in
`ide.c`. Similarly `mknod` creates a device-as-a-file which writes to an inode
which may again sleep in `ide.c`. We now call `mknod` as a system call from
`init.c` after the file system and log are initialized.

The other thing to be careful with is that we don't want to sleep while holding
spinlocks inside `ide.c` and `log.c`. For example, in `log.c`, we want other
processes to be able to call `end_op`, acquire `log.lock`, and commit the log.
Therefore, we evolve the `sleep` interface to additionally take a spinlock. 
The `sleep` implementation releases the lock before sleeping and reacquires it
after waking up.