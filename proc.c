#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

struct {
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void trapret(void);

static void wakeup1(void *chan);

int
cpuid() {
  return 0;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  return &cpus[0];
}

// Read proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c = mycpu();
  return c->proc;
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  pushcli();
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  popcli();
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  popcli();

  if((p->offset = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  p->sz = 0;

  // kstack is its own kalloc'd page. Leave p->kstack as the base of the
  // kalloc'd chunk so kfree(p->kstack) passes kalloc.c's alignment check.
  // We use only the first KSTACKSIZE bytes of the 1MB chunk as the kernel
  // stack; the rest is wasted but the chunk boundary remains aligned.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;                             
  *(uint*)sp = (uint)trapret;          

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  // p->context->eip = (uint)trapret;
  
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first process.
void
pinit(void)
{
  
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  // SHADOW PAGING: Build the initial page directory for initproc.
  p->pgdir = setupkvm();
  inituvm(p->pgdir, _binary_initcode_start, (uint)_binary_initcode_size);

  initproc = p;

  memmove(p->offset, _binary_initcode_start, (int)_binary_initcode_size);
  // p->sz is the size of the user address space, not just the initcode
  // bytes. inituvm mapped one page at virt 0, so argint/argptr must accept
  // user addresses up to PGSIZE (the stack lives at the top of this page).
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));

  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;

  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;   // stack sits at top of the single inituvm'd page
  p->tf->eip = 0;        // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  // cwd is set in forkret, after iinit has initialized the inode cache.

  p->state = RUNNABLE;
}

// process scheduler.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();
    pushcli();
    // Loop over process table looking for process to run.
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process. 
      c->proc = p;
      p->state = RUNNING;

      switchuvm(p);
      swtch(&(c->scheduler), p->context);
    }
    popcli();
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct cpu* c = mycpu();
  struct proc *p = myproc();

  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = c->intena;
  swtch(&p->context, c->scheduler);
  c->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  pushcli();
  myproc()->state = RUNNABLE;
  sched();
  popcli();
}

int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // 1. Allocate a new process from the process table
  if((np = allocproc()) == 0){
    return -1;
  }

  // SHADOW PAGING: Clone the parent's page directory.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }

  // 2. Copy the entire user memory segment
  // In p23's segmentation model, the physical memory starts at np->offset
  // and the size of the user space is curproc->sz
  memmove(np->offset, curproc->offset, PGSIZE - KSTACKSIZE);

  // 3. Copy the trap frame so the child has the exact same register states
  *np->tf = *curproc->tf;

  // 4. Force the child's return value for fork() to be 0 (stored in eax)
  np->tf->eax = 0;

  // 5. Duplicate open file descriptors
  for(i = 0; i < NOFILE; i++) {
    if(curproc->ofile[i]) {
      np->ofile[i] = filedup(curproc->ofile[i]);
    }
  }
  
  // Duplicate the current working directory reference
  if(curproc->cwd) {
    np->cwd = iget(curproc->cwd->dev, curproc->cwd->inum);
  }

  // 6. Set parent process and copy process name
  np->parent = curproc;
  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  // 7. Mark the child as runnable
  pushcli();
  np->state = RUNNABLE;
  popcli();

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  // Release the current working directory
  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  pushcli();

  // Wake up the parent, who might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass any abandoned children to initproc.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();

  pushcli();
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      
      // If we find a ZOMBIE child, clean up its memory!
      if(p->state == ZOMBIE){
        pid = p->pid;
        kfree(p->offset); // Free the 1MB physical block
        p->offset = 0;
        kfree(p->kstack); // Free the kernel stack chunk
        p->kstack = 0;
        // SHADOW PAGING: Free the page directory.
        freevm(p->pgdir);
        p->pgdir = 0;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->state = UNUSED;
        popcli();
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      popcli();
      return -1;
    }

    // Wait for children to exit. (Sleep on our own proc structure)
    sleep(curproc);
  }
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  pushcli();
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      popcli();
      return 0;
    }
  }
  popcli();
  return -1;
}


void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  popcli();

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
    myproc()->cwd = namei("/");
  }

  // Return to "caller", actually trapret (see allocproc).
}


// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan)
{
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  pushcli();
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;
  popcli();
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  pushcli();
  wakeup1(chan);
  popcli();
}

void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie",
  };
  struct proc *p;
  char *state;
  pushcli();
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    cprintf("\n");
  }
  popcli();
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);  // reload CR3 to flush stale TLB entries
  return 0;
}