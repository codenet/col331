## Reading inodes from the file system 

`welcome` method in `main.c` first reads the inode of root directory by
calling `iget` and then `iread`. From this inode we read and print directory
entries for `.`, `..`, and `welcome.txt`.

`welcome.txt` is on the third location after `.` and `..`. We read the inode for
`welcome.txt` file, read its contents into a character array `greet`, and
finally print it.

`struct inode` is defined in `file.h`. It contains everything that the disk 
version of inode `struct dinode` contains. In addition, it also contains the 
device number (for managing multiple disks), the inode number, number of
references to this in-memory `inode` struct, and if it is valid i.e, if it has
been read from the disk.

`fs.c` maintains a cache of inodes. In a fuller OS, some directories such as the
root will be read again and again so we don't want to read them back from disk
every time. When number of references to in-memory copy of inode become zero,
they can be evicted from the cache. This number of references is not to be
confused with `dinode.nlink` which is the number of links to this `inode` in the
file system. Evicting an in-memory `inode` from `icache` does not delete its disk 
copy.

Before jumping to `welcome`, the `main` method called `iinit` that "initializes"
the file system by reading the super block.

`fs.c` additionally implements the following methods:

* `iget` first looks up the inode in cache and return a new inode if it is
not found.
* `iread` reads the inode from disk if it isn't already read.
* `stati` reads the header of `inode`.
* `readi` reads the contents of `inode`. `readi` reads data blocks one-by-one.
It takes help from `bmap` to figure out which data blocks overlap with the read
request.