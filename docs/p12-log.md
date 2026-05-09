## Crash consistency

One of the most interesting parts of working with persistent storage like disk
is "crash consistency". The issue is that a computer can stop working at any 
point of time: maybe the power got cut or maybe the kernel had a bug somewhere
(independent of the file system) and just crashed. If the disk is removable like
a USB drive, the user might randomly pull out the drive at any time.

Due to such behaviors, our disk may end up in an "inconsistent" state. For
instance, `mkdir` calls `create` that calls `ialloc` to allocate an `inode` for
the new directory. If we lose power at this point, we end up with an allocated
`inode` and no directory information in it. This `inode` may never be used for 
any other file or directory.

This problem is relatively easier to fix. At the time of boot, we can scan all
the inodes. If an inode or a data node is not reachable from the root directory,
then it can be marked as free.

In fact, some kernels did this by running a program called `fsck`. However,
`fsck` may take a long time to complete especially for large disks thereby
slowing down the boot. More critically, `fsck` may not be able to fix all
inconsistencies.

xv6 uses *write ahead logging (WAL)* via *redo logs*, implemented in `log.c`,
for crash consistency. The same approach is also used in many databases.
Basically, whenever we are writing multiple disk blocks to service a call like
`mkdir`, we want to make the modifications to be *atomic*. Atomicity means that
either *all* of the writes will make it to the disk or *none* of the writes will.

Instead of writing disk blocks directly, we first write them to a log. When all 
the writes are done for the call, we mark the log as committed, and write the
disk blocks to their real position.

## Code walkthrough

In particular, in `fs.c` all `bwrite` are replaced by `log_write` and in
`file.c`, we put operations between `begin_op` and `end_op`. These are defined
by `log.c`. 

The log is a list of blocks after the superblock (see `mkfs.c`). The structure
of the log is that we first have a header block (defined by `struct logheader`)
that contains number of active blocks in the log and the block number where each
block in the log needs to be written to. To begin with, logheader says that
there are zero log blocks.

Each call to `log_write` is just remembering the block number that is being
written by the call. It also marks the block as dirty, so that the buffer cache
layer does not evict the block.  When `end_op` is called, it calls `commit` to
write the blocks changed by the operation. `commit` first calls `write_log` that
writes all the blocks to the log, that were modified by `log_write` calls. Then,
it updates the log header about the block information in the log.  

This is where the operation is considered **committed**. If we reboot after this
point, the changes made by the operation will be applied. We will see this later
in `initlog`. 

`commit` then calls `install_trans` to write the modified blocks in their correct
places. Finally, it resets the log header block.

After reboot, `main.c` calls `initlog` which calls `recover_from_log`. This 
calls `read_head` to read the header block to see if there are any pending
blocks in the log that are to be written. Header block only contains committed
operations that we were not finished writing. `install_trans` writes the blocks
from the log to their appropriate places and finally clears the header block. 

This scheme works because writing blocks is **idempotent**. It is possible that
the computer lost power just before clearing the log header block, i.e, the
blocks were already written before crash. Rewriting them after reboot does not 
matter since it writes the same content on the same block.