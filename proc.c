#include "types.h"
#include "defs.h"
#include "x86.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "param.h"
#include "stat.h"
#include "file.h"
#include "fcntl.h"
#include "mmu.h"
#include "proc.h"
#include "memlayout.h"


struct {
  struct spinlock lock;
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

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock); // (if no slots found)
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock); // (safe to release now that state is EMBRYO)

  if((p->offset = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  p->sz = PGSIZE - KSTACKSIZE;

  sp = (char*)(p->offset + PGSIZE);

  // Allocate kernel stack.
  p->kstack = sp - KSTACKSIZE;

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
  initlock(&ptable.lock, "ptable");
  
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;

  memmove(p->offset, _binary_initcode_start, (int)_binary_initcode_size);
  memset(p->tf, 0, sizeof(*p->tf));

  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;

  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE - KSTACKSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

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

    acquire(&ptable.lock); // (lock before scanning processes)
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
    release(&ptable.lock); // (release when done scanning)
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
  acquire(&ptable.lock);
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
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

  // 7. Acquire the lock and mark the child as runnable
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);

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

  acquire(&ptable.lock);

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

  acquire(&ptable.lock);
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
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        // p->killed = 0; // Uncomment if your struct proc has a killed field
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids){ // Add `|| curproc->killed` if your proc.h has a killed field
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit. (Sleep on our own proc structure)
    sleep(curproc, &ptable.lock);  
  }
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}


void
forkret(void)
{
  static int first = 1;
  static volatile int init_done = 0; // <-- New global flag
  int is_first;

  // 1. Check the flag while we hold the scheduler lock
  is_first = first;
  if(first)
    first = 0;

  // 2. Safe to release the lock now
  release(&ptable.lock);

  // 3. Process 1 does the heavy lifting
  if (is_first) {
    iinit(ROOTDEV);
    initlog(ROOTDEV);
    struct inode console;
    mknod(&console, "console", CONSOLE, CONSOLE);
    
    init_done = 1; // <-- Signal that the disk is ready!
  } else {
    // 4. Any other process must patiently wait
    while(init_done == 0) {
      yield(); // Give the CPU back to the OS until Process 1 finishes
    }
  }

  // Return to "caller", actually trapret (see allocproc).
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
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){
    acquire(&ptable.lock);
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched(); // Yield the CPU

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){
    release(&ptable.lock);
    acquire(lk);
  }
}

int
growproc(int n)
{
  struct proc *p = myproc();
  int newsz;

  newsz = (int)p->sz + n;
  if(newsz < 0)
    return -1;
  if(newsz >= PGSIZE - KSTACKSIZE)
    return -1;

  if(n > 0)
    memset(p->offset + p->sz, 0, n);

  p->sz = newsz;
  return 0;
}

void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [SLEEPING]  "sleep ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;
  acquire(&ptable.lock);
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
  release(&ptable.lock);
}