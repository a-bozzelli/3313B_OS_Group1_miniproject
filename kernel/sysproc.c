#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * System call implementation for retrieving per-process
 * cost information from the kernel.
 *
 * int proccost(struct proccostinfo *info, int max)
 *   - info: user pointer to an array of proccostinfo
 *   - max:  maximum number of entries the user can store
 *
 * Returns the number of populated entries on success,
 * or -1 on error. The cost field in each entry is
 * computed as:
 *   cost = cpu_ticks + 5 * disk_writes
 * ========================================================= */

// ===== FEATURE 2 START: Expensive Process Analysis =====

uint64
sys_proccost(void)
{
  uint64 uaddr;
  int max;

  // Fetch syscall arguments: user buffer address and max count.
  // argaddr/argint do not return a status in this xv6 variant,
  // so we simply call them and then validate the values.
  argaddr(0, &uaddr);
  argint(1, &max);
  if(max <= 0)
    return -1;

  struct proc *p = myproc();
  struct proccostinfo local[NPROC];
  int count = 0;

  // Walk the process table and snapshot accounting data
  // while holding each process's lock to get a consistent
  // view of its fields.
  for(struct proc *q = proc; q < &proc[NPROC] && count < max; q++) {
    acquire(&q->lock);
    // Only report meaningful process entries. Skip slots that
    // are completely unused or merely reserved (USED), since
    // they do not correspond to runnable/sleeping/zombie
    // processes with interesting accounting data.
    if(q->state != UNUSED && q->state != USED) {
      local[count].pid = q->pid;
      safestrcpy(local[count].name, q->name, sizeof(local[count].name));
      local[count].cpu_ticks = q->cpu_ticks;
      local[count].sched_count = q->sched_count;
      local[count].disk_writes = q->disk_writes;

      // Simple cost model combining CPU and disk usage.
      local[count].cost = local[count].cpu_ticks +
                          5 * local[count].disk_writes;

      count++;
    }
    release(&q->lock);
  }

  int bytes = count * sizeof(struct proccostinfo);
  if(bytes > 0) {
    if(copyout(p->pagetable, uaddr, (char*)local, bytes) < 0)
      return -1;
  }

  return count;
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
