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
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, STARTPROC, (PROCSIZE << 12), DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, STARTPROC, (PROCSIZE << 12), DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}