#include "types.h"
#include "mmu.h"
#include "x86.h"
#include "param.h"
#include "proc.h"
#include "defs.h"
#include "memlayout.h"

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, STARTPROC, PROCSIZE, 0);
  c->gdt[SEG_UDATA] = SEG(STA_W, STARTPROC, PROCSIZE, 0);
  lgdt(c->gdt, sizeof(c->gdt));
}