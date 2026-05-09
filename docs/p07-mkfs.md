## File system

File system is a basically an on-disk linked data structure (tree) with
directories pointing to other directories and files. We need to define how the
file contents and pointers are managed over disk blocks to define our file
system.

`struct dinode` in `fs.h` specifies how an inode is represented on disk. One
`dinode` is of size 64 bytes. Therefore we can fit 8 inodes in a disk block.
`mkfs.c` prepares `fs.img` file from `welcome.txt` file.  When we open `fs.img`
in a hex editor, we see: 

```
00000200: e803 0000 cb03 0000 c800 0000 0000 0000
00000210: 0200 0000 0200 0000 1c00 0000 0000 0000
```

This is the `struct superblock`:
* `size` = `0x03e8` = 1,000 (`FSSIZE`). This indicates the number of blocks in
  the file system.
* `nblock`=`0x03cb` = 971. This indicates the number of available data blocks.
  (more below).
* `ninodes`=`0xc8` = 200. Hence, inodes are present in 200/8 = 25 blocks.
* `nlog` = 0, `logstart`=2. Ignore this for now.
* `inodestart`=2. This is saying that inodes start from the second block (right
  after the superblock).
* `bmapstart`=`0x1c`=28. Free bit map starts after all the 25 inodes.

1 boot block + 1 super block + number of data blocks + number of inode blocks
(25) + number of bitmap blocks (2) = total number of blocks (1000). 

Bitmap blocks spend 1 bit to keep track of free data blocks. For 1000 total
blocks, we need 1000 bits, i.e, 125 bytes. This can be represented in just 1 512
byte block. But `mkfs.c` sets `nbitmap` to 2.

So we get 971 data blocks. Data blocks start at block number 29, after free bit
map blocks.


```
00000440: 0100 0000 0000 0100 0002 0000 1d00 0000
```

This is the 64 bytes of the second inode representing the "/" directory. First
two bytes is `0100` which is 1 in little-endian. This means that this file is a
directory (`type` is `T_DIR`).  `major` and `minor` are both zero, `nlink` is 1.
`nlink`>0 signifies that this inode is "live" and is not part of a deleted file.
We might later see that `nlink` can be greater than 1 when we create symbolic
links to the same file. Notice that `.` and `..` do not count towards `nlink`; 
otherwise a directory may never get deleted.

`size` is `0002 0000` which is basically `0x0000 0200` = 512 bytes. The first
address is `1d00 0000` which is basically `0x1d` = 29 (the first data block).

The next 64 bytes represent the "welcome.txt" file.
```
00000480: 0200 0000 0000 0100 a701 0000 1e00 0000
```

Here the type is `0200`=`0x2` representing a file. The size of the file is
`a7010000` = `0x01a7` = 423 bytes (check `wc welcome.txt`). It also has one 
data block `0x1e`=30.

Now, we can read the data blocks from `fs.img`.  29th block starts at address
29*512 = `0x3A00`. 
```
00003a00: 0100 2e00 0000 0000 0000 0000 0000 0000  ................
00003a10: 0100 2e2e 0000 0000 0000 0000 0000 0000  ................
00003a20: 0200 7765 6c63 6f6d 652e 7478 7400 0000  ..welcome.txt...
00003a30: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```

This is the "/" directory. A directory is essentially a file containing a
sequence of directory entries defined in `struct dirent`. The `dirent` occupies
16 bytes so in one data block we can store 512/16 = 8 directory entries. Notice 
that we will not allow specifying file and directory names to be greater than 14
characters. The first two directory entries map `.` and `..` (0x2e=. in ASCII)
to inode number 1 which is the same as the "/" directory.  The third directory
entry maps "welcome.txt" file to inode number 2.

Now, we read the data blocks for `welcome.txt` at data block = 30.

```
00003c00: 2020 2020 2020 2020 2020 2020 2020 2020
00003c10: 2020 2020 2020 2020 2020 2020 2020 2020
00003c20: 2020 2020 2020 2020 2020 2020 2020 2020
00003c30: 2020 2020 2020 2020 2020 2323 230a 2023            ###. #
00003c40: 2020 2020 2320 2023 2323 2323 2320 2023      #  ######  #
```

We see that spaces, dots, and hash signs are in the data block of the file!