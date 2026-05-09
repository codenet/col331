## Memory segmentation

Now before we can run our first process, we should understand memory segments a
bit better. Memory segments split physical memory into contiguous segments.
Different segments can serve different purposes, like belonging to different
processes, etc. Section 3.2 of Intel SDM Volume 3A describes three different 
segmentation approaches: "basic flat model", "protected flat model" and
"multi-segment model".

For simplicity, xv6 implements the "basic flat model". There are three main
segment registers also called segment selectors: code segment (CS), stack
segment (SS), data segment (DS); and three additional data segment registers
which are not used that much: ES, FS, and GS. Segment registers are essentially
indices into Descriptor Tables (Global descriptor table GDT or Local descriptor
table LDT) which describe the segment. LDT is not used by xv6. The layout of
segment register is shown in Figure 3.6 of Intel SDM Volume 3A.

GDT was setup by `bootasm.S` as per the "basic flat model". GDT is just a
location in memory pointed to by the GDTR register. GDTR register is modified
using the `lgdt` instruction. Therefore, GDT is very much like IDT. But, GDT
describes segment information whereas IDT described interrupt information.

Every memory access goes through an address translation mechanism as shown in
Figure 3.5. A logical address in `EIP` register is translated using the code
segment. All stack based instructions use the stack segment. For example, the
`push` and `pop` instructions and setting/getting `ESP` and `EBP` registers.
Other memory instructions such as `mov`, `lea` etc are translated using the data
segment.

Since the contents of GDT are used repeatedly in every address translation, the 
contents are cached within the segment register. Therefore, segment registers
have a visible portion (controlled by the OS) and a hidden portion (cached from
GDT). This is done so that we don't need to read the GDT at every memory access.

When we use `info registers` in QEMU's monitor mode, we see both the visible
portion and the hidden portion. For example, in 
`CS =0008 00000000 ffffffff 00cf9a00`, the visible portion is `0008` 
as per Figure 3.6; i.e, `CS` is pointing to the first entry of GDT. The rest
`00000000 ffffffff 00cf9a00` is the hidden portion cached from GDT, used for
address translation.

`info registers` also shows `GDT=00007c60 00000017`. This means that the base of
GDT is at `7c60` and the size is `0x17=23`. Each entry in GDT is of 8 bytes.
There are three entries; size locates the last bit of GDT which will be at GDT
base + 8*3-1(=23). Now in gdb, we can run `x /24xb 0x7c60` to see 24 bytes
starting at the base address of GDT.

```
(gdb) x /24xb 0x00007c60
0x7c60: 0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0x7c68: 0xff    0xff    0x00    0x00    0x00    0x9a    0xcf    0x00
0x7c70: 0xff    0xff    0x00    0x00    0x00    0x93    0xcf    0x00
```

We see that the first 8 bytes are all zero corresponding to the "null segment".
There are two memory segments: one for code and another for data. Both segments
have access to full 4GB of memory: start is set to zero and limit is set to 4GB.

Code segment is `0xff 0xff 0x00 0x00 0x00 0x9a 0xcf 0x00`. The format of a 
segment descriptor is described in Figure 3.8. Remember that x86 is
little-endian.

1. 2 least significant bytes represent two least significant bytes of limit:
`0xffff`.
2. Base address is split across the descriptor. Patching them together, we get
based address of the segment as `0x00000000`.
3. The remaining thing is `0x9a 0xcf`. From this, we get:
0xa
* Type: 1010. From Table 3.1, means that this is a code segment which can be read
  and executed.
0x9
* P: 1. Segment is present.
* DPL: 00. Means that it is a segment owned by privilege level=0.
* S: 1. Means it is a code/data segment.
0xf
* Seg limit: 1111. 
0xc
* G: 1. Signifies that we are treating limit in a Granularity of 4KB. So the
segment goes up to 4GB.
* D/B: 1. Means we are executing in 32 bit mode.
* L: 0. We are not executing in 64-bit mode. 
* AVL: 0. Reserved bit.

We prepare for running our first process by setting up two more memory segments
in `vm.c`. Since our kernel is small, it fits in the first 2MB. We reserve 2MB
to 3MB area for our first process.
