## Programmable Interrupt Controller (PIC)

Original design of PIC (8259A) had dedicated pins for receiving interrupts from
different devices. One PIC could get 8 interrupts corresponding to 8 pins.  They
were connected in a primary-secondary fashion where the secondary PIC is
attached to the primary PIC's pin 2 and the primary PIC is attached directly to
the CPU. These interrupt pins were labelled IRQ0 to IRQ7 for primary, and IRQ8 to
IRQ15 for the secondary. In total, we can effectively get 15 (16 minus primary's
pin 2) interrupt pins.

By a matter of convention, timer is attached on IRQ0, keyboard on IRQ1,
real-time clock on IRQ8, mouse on IRQ12, IDE controller on IRQ14, etc. Some of
these IRQs are defined in `traps.h`. However, since there were only 15 pins,
some devices started sharing the pins which created complications in managing
the interrupts. Moreover, PICs did not have support for multi-core processors.
Therefore, Intel introduced advanced programmable interrupt controller (APICs).
After boot, `main.c` calls `picinit` in `picirq.c` which disables PICs in favor
of using APICs.

## Advanced programmable interrupt controller (APIC)

To support multi-core processors, there is now a local APIC for each core, that
can be independently interrupted, such as timer interrupts, and a common IO APIC
which is interrupted by devices such as keyboard, mouse, and disk controller.
See Figure 10.3 Intel SDM Volume 3, Part 1. Local APICs (LAPICs) also forward
and receive inter-processor interrupts (IPI).

Table 10.1 Intel SDM Volume 3, Part 1 describes all the registers in LAPIC.
`main.c` calls `lapicinit` which enables LAPIC, sets periodic timer interrupts, 
acknowledges any outstanding interrupts by resetting end of interrupt (EOI), and
enables all external interrupts by setting task priority register (TPR) to 0.
For example, TPR=8 would have blocked interrupts with priority class < 9.
Section 10.8.3 Intel SDM Volume 3, Part 1 discusses interrupt priorities. Note
that this does not enable interrupts at CPU. You can [read
here](https://wiki.osdev.org/APIC_Timer#Periodic_Mode) about how the timer
interrupt period is decided from LAPIC registers.

Notice that we were able to set LAPIC registers by just setting an `index` in
`lapicw` instead of using the `out` instruction (defined in `outb` method in
`x86.h`). We need to use the `out` instruction for "port-mapped IO" that
reads/writes data on a port. We can just read/write a memory location for
"memory-mapped IO". LAPIC supports memory-mapped IO. Our hardware understands 
that when we are trying to write to memory locations "0xfe000000..0xffffffff",
we are actually trying to write to a device.

In a similar manner, IOAPICs are also memory-mapped, located at `0xFEC00000`.
`ioapic.c` just reads how many interrupt pins are on the IOAPIC in `maxintr`
variable and disables them. They will be later enabled when we are ready to 
handle interrupts. You can read more about IOAPICs
[here](https://web.archive.org/web/20161130153145/http://download.intel.com/design/chipsets/datashts/29056601.pdf).