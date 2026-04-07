// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];
// NOTE: the global `proc` array is declared after `struct proc` is defined.

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

enum powermode { POWER_NORMAL = 0, POWER_ECO = 1 };

#define ECO_SKIP_INTERVAL 3
#define CPU_BUDGET_WINDOW_TICKS 50

// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  // power restriction fields (protected by p->lock)
  int power_mode;              // POWER_NORMAL or POWER_ECO
  int cpu_budget;              // budget in ticks per window; 0 means unlimited
  int ticks_used;              // ticks consumed in current window
  int ticks_total;             // lifetime tick usage (for reporting/demo)
  uint budget_window_start;    // global tick when accounting window started
  int eco_skip_counter;        // scheduler-side counter for eco mode cadence
  /* =========================================================
   * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
   * Per-process accounting fields used for resource cost tracking.
   * These counters are updated in clearly marked FEATURE 2 blocks
   * to keep this feature easy to identify and merge.
   * ========================================================= */

  // ===== FEATURE 2 START: Expensive Process Analysis =====

  // Total number of timer ticks during which this process was RUNNING.
  // This is a coarse measure of CPU usage over the lifetime of the process.
  uint64 cpu_ticks;

  // Optional hard CPU usage cap (0 means unlimited).
  uint64 cpu_ticks_limit;

  // Number of times the scheduler chose this process to run.
  // This helps quantify how often the process has been scheduled.
  uint64 sched_count;

  // Approximate count of disk-backed write system calls performed
  // by this process. We increment this once per write() that targets
  // an inode-backed file (FD_INODE), which is a reasonable proxy for
  // disk writes without invasive changes to the file system code.
  uint64 disk_writes;

  // ===== FEATURE 2 END: Expensive Process Analysis =====
};

/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * Kernel-visible structure used to return per-process cost
 * information to user space via the proccost() system call.
 * A matching definition exists in user/user.h and must stay
 * in sync for the syscall ABI to remain correct.
 * ========================================================= */

// ===== FEATURE 2 START: Expensive Process Analysis =====

struct proccostinfo {
  int    pid;                 // Process ID
  char   name[16];            // Process name (truncated to 15 chars + NUL)

  uint64 cpu_ticks;           // CPU ticks while RUNNING
  uint64 sched_count;         // Number of times scheduled
  uint64 disk_writes;         // Approximate disk-backed write() count

  // Simple linear cost score combining CPU and disk usage.
  // Currently: cost = cpu_ticks + 5 * disk_writes
  uint64 cost;
};

// ===== FEATURE 2 END: Expensive Process Analysis =====

// The global process table lives in proc.c but is
// referenced by some FEATURE 2 code (e.g., sys_proccost).
// Declare it here for kernel files that need to walk it.
extern struct proc proc[];
