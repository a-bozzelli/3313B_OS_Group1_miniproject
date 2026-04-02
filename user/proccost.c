/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User program: proccost
 *
 * This program queries the kernel for per-process cost data
 * via the proccost() system call and prints the most
 * expensive processes in a simple table.
 * ========================================================= */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// ===== FEATURE 2 START: Expensive Process Analysis =====

// Maximum number of processes we expect. This should be
// at least as large as NPROC in the kernel.
#define MAX_PROCS 64

// Simple in-place sort by descending cost score.
static void
sort_by_cost(struct proccostinfo *info, int n)
{
  for(int i = 0; i < n; i++) {
    int max_idx = i;
    for(int j = i + 1; j < n; j++) {
      if(info[j].cost > info[max_idx].cost) {
        max_idx = j;
      }
    }
    if(max_idx != i) {
      struct proccostinfo tmp = info[i];
      info[i] = info[max_idx];
      info[max_idx] = tmp;
    }
  }
}

int
main(int argc, char *argv[])
{
  struct proccostinfo info[MAX_PROCS];

  // Ask the kernel to fill in per-process accounting data.
  int n = proccost(info, MAX_PROCS);
  if(n < 0) {
    printf("proccost: syscall failed\n");
    exit(1);
  }
  if(n > MAX_PROCS)
    n = MAX_PROCS;

  // Sort processes by descending cost so the
  // most expensive ones appear first.
  sort_by_cost(info, n);

  // Print a simple header and one line per process.
  printf("PID   NAME            CPU_TICKS   SCHED_COUNT   DISK_WRITES   COST\n");
  for(int i = 0; i < n; i++) {
    printf("%d    %s\t\t%lu\t%lu\t%lu\t%lu\n",
           info[i].pid,
           info[i].name,
           info[i].cpu_ticks,
           info[i].sched_count,
           info[i].disk_writes,
           info[i].cost);
  }

  exit(0);
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
