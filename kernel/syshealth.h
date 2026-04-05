// Shared struct for sys_getsyshealth syscall.
// Used by both kernel (sysproc.c) and userspace (syshealth.c).

struct syshealth {
  uint uptime_ticks;      // total timer ticks since boot
  uint active_ticks;      // ticks a process was RUNNING (not idle)
  uint idle_ticks;        // ticks no process was running
  uint context_switches;  // total scheduler context switches
  uint nproc_running;     // processes currently RUNNING or RUNNABLE
  uint nproc_total;       // total non-UNUSED process slots in use
  uint free_pages;        // free physical memory pages
};
