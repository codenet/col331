#include "types.h"
#include "memlayout.h"
#include "asm.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  lgdt(c->gdt, sizeof(c->gdt));
}

void
switchuvm(struct proc *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");

  pushcli();
  mycpu()->gdt[SEG_UCODE] = SEG(STA_X|STA_R, p->offset, (PROCSIZE << 12)-1, DPL_USER);
  mycpu()->gdt[SEG_UDATA] = SEG(STA_W, p->offset, (PROCSIZE << 12)-1, DPL_USER);
  lgdt(mycpu()->gdt, sizeof(mycpu()->gdt));

  mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
                                sizeof(mycpu()->ts)-1, 0);
  mycpu()->gdt[SEG_TSS].s = 0;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (ushort) 0xFFFF;
  ltr(SEG_TSS << 3);
  popcli();
}