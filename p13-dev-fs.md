## Interacting with devices like files

One cool thing about xv6 (and Linux) is that many resources are exposed as
files. `console.c` just defines `consoleread` and `consolewrite` methods and map
them to the `struct devsw` to expose the console device as a file. 

`main.c` calls `consoleinit` to let `console.c` map the functions and then calls
`mknod` to create an inode that works like the file for the console device.
After this call, the console device becomes available with a filename "console".
`welcome` method in `main.c` just uses file interfaces like `open`, `fileread`,
and `filewrite` to read and write to console!

On a Linux terminal, you can run `tty` to get a "file name" like `/dev/pts/17`,
and then do `cat /dev/pts/17`. Now, when you type something and press enter
(which does a filewrite to the console device), you should see the same output
being repeated (from the `cat` command reading the filewrite). You can also
play with trying to read the contents of another terminal. You can also explore
`/proc` and `/sys`. Both of these are virtual file systems letting you interact
with different parts of the OS via file interface.