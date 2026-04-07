#include "kernel/types.h"
#include "kernel/syshealth.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int verbose = 0;
  if(argc > 1 && strcmp(argv[1], "-v") == 0)
    verbose = 1;
  (void)argv;

  struct syshealth sh;

  if(getsyshealth(&sh) < 0){
    fprintf(2, "syshealth: syscall failed\n");
    exit(1);
  }

  if(verbose){
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
  } else {
    printf("uptime=%d active=%d idle=%d ctxsw=%d run=%d total=%d free_pages=%d\n",
           sh.uptime_ticks, sh.active_ticks, sh.idle_ticks, sh.context_switches,
           sh.nproc_running, sh.nproc_total, sh.free_pages);
  }

  exit(0);
}
