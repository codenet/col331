# Sleep locks

The approach of disabling interrupts for mutual exclusion works fine on a single
CPU for *small critical sections* that can run quickly. However, often times we
need mutual exclusion across *large critical sections*. 

Consider `bio.c`. We want to synchronize accesses to buffers from multiple
processes; only a single process shall be able to read/write a given buffer at a
time. Critical sections in `bio.h` are long: a process can hold a buffer for a
long time and reading/writing a buffer can sleep in `ide.c`.  Therefore, it does
not make sense to disable interrupts for the long time during which a process is
working with a buffer.

For such *large* critical sections, it is better to acquire *sleep locks*. When a
sleep lock is acquired, its `locked` flag is simply set to `1`; release sets it
to `0`. While acquiring, if we find that the `locked` flag is already set, the
process just sleeps on the lock. It gets woken up when the sleep lock is released.

`bio.c` acquires the buffer's sleep lock in `bget` and release it in `brelse`.
Similarly, inodes are synchronized using sleep locks.