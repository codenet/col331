## Talking to disk

Now we will start implementing a "file system". We will later use this file
system to store some executables and run them.  First thing we need to do for
the file system is to be able to talk to the disk. By running `make fs`, we
create a simple disk image with just two sectors. First sector has a welcome
text, second sector is just zeros.

`bio.c` adds a *buffer cache* layer: a space in memory to read and write blocks
between disk and memory. It has two functions: `bread` and `bwrite`. Since disk 
is slow and since there may be locality of disk reads and writes, we don't want
to go to disk again and again. `bread` first checks if the block we are trying 
to read is already in the cache. If it is in the buffer cache, then we can just
return from cache. Otherwise, we call `iderw` to read the block.

`iderw` just read and write the block. `main.c` reads both blocks. It just
prints the first block and maintains a count of reboots in the second block.
Maintaining this count works because disk is *persistent storage* unlike memory.
