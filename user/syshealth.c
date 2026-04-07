#include "kernel/types.h"
#include "kernel/syshealth.h"
#include "user/user.h"

int
main(void)
{
  struct syshealth sh;

  if(getsyshealth(&sh) < 0){
    fprintf(2, "syshealth: syscall failed\n");
    exit(1);
  }

  printf("=== xv6 System Health Report ===\n");
  printf("Uptime (ticks)      : %d\n", sh.uptime_ticks);
  printf("Active CPU ticks    : %d\n", sh.active_ticks);
  printf("Idle CPU ticks      : %d\n", sh.idle_ticks);
  printf("Context switches    : %d\n", sh.context_switches);
  printf("Procs (run/ready)   : %d\n", sh.nproc_running);
  printf("Procs (total)       : %d\n", sh.nproc_total);
  printf("Free pages          : %d\n", sh.free_pages);
  printf("Free memory (KB)    : %d\n", sh.free_pages * 4);
  printf("================================\n");

  exit(0);
}
