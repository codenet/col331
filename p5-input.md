## Receiving keyboard input

Self study.

Tips:

* You have to be very careful about calling `lapiceoi` in trap handling
routines, otherwise the future interrupts will get blocked.

* Notice that in `ioapicenable` we can specify which CPU should an interrupt be
routed towards.  We ask IOAPIC to send serial port interrupts to CPU 0.
Currently we only have a single CPU configured in `Makefile`. Later we might ask
IOAPIC to route all the disk interrupts to CPU 1. This makes sense for IOAPIC 
since it is shared by CPUs whereas LAPIC does not have routing functionality
since it is per-CPU. OS may choose to not enable timer interrupts, for example,
in LAPIC of some CPUs.

* You can get to "QEMU monitor" by pressing `Ctrl+A C`. 

* In the QEMU monitor, you can run `info irq` to see the number of times
interrupts were raised. Note that our trap handler may not be getting called for
some of these interrupts as they may not be enabled.

* In the QEMU monitor, you can
run `info pic` to see which interrupts are enabled in IOAPIC and run `info
lapic` to see the LAPIC registers.