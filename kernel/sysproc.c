#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "syshealth.h"

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

uint64
sys_setpowermode(void)
{
  int pid;
  int mode;

  argint(0, &pid);
  argint(1, &mode);
  return setpowermode_pid(pid, mode);
}

uint64
sys_setcpulimit(void)
{
  int pid;
  int limit;

  argint(0, &pid);
  argint(1, &limit);
  return setcpulimit_pid(pid, limit);
}

uint64
sys_getcputicks(void)
{
  int pid;

  argint(0, &pid);
  return getcputicks_pid(pid);
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
  int count = 0;
  struct proccostinfo entry;

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
      entry.pid = q->pid;
      safestrcpy(entry.name, q->name, sizeof(entry.name));
      entry.cpu_ticks = q->cpu_ticks;
      entry.sched_count = q->sched_count;
      entry.disk_writes = q->disk_writes;

      // Simple cost model combining CPU and disk usage.
      entry.cost = entry.cpu_ticks + 5 * entry.disk_writes;

      // Release the process lock before copying out to user space,
      // since copyout may sleep.
      release(&q->lock);

      // Copy this entry directly into the user-provided array at
      // the appropriate offset.
      if(copyout(p->pagetable,
                 uaddr + count * sizeof(struct proccostinfo),
                 (char *)&entry,
                 sizeof(struct proccostinfo)) < 0) {
        return -1;
      }

      count++;
      continue;
    }
    release(&q->lock);
  }
  return count;
}

// ===== FEATURE 2 END: Expensive Process Analysis =====

// Fill a struct syshealth with current system statistics.
uint64
sys_getsyshealth(void)
{
  uint64 addr;
  argaddr(0, &addr);

  struct syshealth sh;

  // Snapshot tick counters atomically
  acquire(&tickslock);
  sh.uptime_ticks = ticks;
  sh.active_ticks = active_ticks;
  sh.idle_ticks   = idle_ticks;
  release(&tickslock);

  sh.context_switches = context_switches;
  sh.free_pages       = kfreepages();

  // Count process states
  sh.nproc_running = 0;
  sh.nproc_total   = 0;
  extern struct proc proc[];
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED) continue;
    sh.nproc_total++;
    if(p->state == RUNNING || p->state == RUNNABLE)
      sh.nproc_running++;
  }

  // Copy struct out to userspace
  if(copyout(myproc()->pagetable, addr,
             (char *)&sh, sizeof(sh)) < 0)
    return -1;
  return 0;
}

// set usage limit for a process (max CPU ticks)
uint64
sys_setusagelimit(void)
{
  int pid;
  int max;
  argint(0, &pid);
  argint(1, &max);

  for(struct proc *p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->cpu_ticks_limit = (max < 0) ? 0 : (uint64)max;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// get usage stats for a process: copy {cpu_ticks, cpu_ticks_limit} to user addr
uint64
sys_getusagestats(void)
{
  int pid;
  uint64 addr;
  argint(0, &pid);
  argaddr(1, &addr);

  struct {
    uint64 cpu_ticks;
    uint64 cpu_ticks_limit;
  } stats;

  for(struct proc *p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      stats.cpu_ticks = p->cpu_ticks;
      stats.cpu_ticks_limit = p->cpu_ticks_limit;
      release(&p->lock);
      if(either_copyout(1, addr, (char *)&stats, sizeof(stats)) < 0)
        return -1;
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}
