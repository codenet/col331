// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
};

extern struct cpu cpus[NCPU];
extern int ncpu;