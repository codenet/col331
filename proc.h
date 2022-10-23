// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
};

extern struct cpu cpus[NCPU];
extern int ncpu;