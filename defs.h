struct buf;
struct rtcdate;
struct superblock;
struct inode;
struct stat;
struct context;
struct proc;

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            bwrite(struct buf*);
void            brelse(struct buf *b);

// console.c
void            cprintf(char*, ...);
void            halt(void) __attribute__((noreturn));
void            panic(char*) __attribute__((noreturn));
void            consoleintr(int(*)(void));
void            consoleinit(void);
void            consputc(int);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);
int             mkdir(char *path);
struct file*    open(char* path, int omode);
int             unlink(char* path, char* name);
int             isdirempty(struct inode *dp);
int             mknod(struct inode *ip, char* path, int major, int minor);

// fs.c
void            readsb(int dev, struct superblock *sb);
void            iinit(int dev);
void            iread(struct inode*);
struct inode*   iget(uint dev, uint inum);
int             readi(struct inode*, char*, uint, uint);
void            stati(struct inode*, struct stat*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
struct inode*   dirlookup(struct inode*, char*, uint*);
int             dirlink(struct inode *dp, char *name, uint inum);
struct inode*   ialloc(uint dev, short type);
void            iupdate(struct inode *ip);
void            iput(struct inode*);
int             writei(struct inode*, char*, uint, uint);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);
extern uchar    ioapicid;
void            ioapicinit(void);

// kalloc.c
char*           kalloc(void);
void            kfree(char*);
void            kinit(void*, void*);

// lapic.c
int             lapicid(void);
extern volatile uint*    lapic;
void            lapiceoi(void);
void            lapicinit(void);
void            lapicstartap(uchar, uint);
void            microdelay(int);

// log.c
void            initlog(int dev);
void            log_write(struct buf*);
void            begin_op();
void            end_op();

// mp.c
extern int      ismp;
void            mpinit(void);

// picirq.c
void            picenable(int);
void            picinit(void);

// proc.c
int             cpuid(void);
void            pinit(void);
struct cpu*     mycpu(void);
struct proc*    myproc();
void            scheduler(void) __attribute__((noreturn));
void            procdump(void);
void            yield(void);

// spinlock.c
void            getcallerpcs(void*, uint*);
void            pushcli(void);
void            popcli(void);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// swtch.S
void            swtch(struct context**, struct context*);

// trap.c
void            idtinit(void);
extern uint     ticks;
void            tvinit(void);

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);

// vm.c
void            seginit(void);
void            switchuvm(struct proc*);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
