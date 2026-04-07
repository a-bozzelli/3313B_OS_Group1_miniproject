#include "kernel/types.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage:\n");
  fprintf(2, "  powerctl <pid> <normal|eco>\n");
  fprintf(2, "  powerctl <pid> limit <ticks_per_window>\n");
  fprintf(2, "  powerctl <pid> stats\n");
}

int
main(int argc, char **argv)
{
  if(argc < 3){
    usage();
    exit(1);
  }

  int pid = atoi(argv[1]);
  if(pid <= 0){
    fprintf(2, "powerctl: invalid pid '%s'\n", argv[1]);
    exit(1);
  }

  if(strcmp(argv[2], "normal") == 0){
    if(setpowermode(pid, 0) < 0){
      fprintf(2, "powerctl: setpowermode failed for pid %d\n", pid);
      exit(1);
    }
    printf("pid %d power mode set to normal\n", pid);
    exit(0);
  }

  if(strcmp(argv[2], "eco") == 0){
    if(setpowermode(pid, 1) < 0){
      fprintf(2, "powerctl: setpowermode failed for pid %d\n", pid);
      exit(1);
    }
    printf("pid %d power mode set to eco\n", pid);
    exit(0);
  }

  if(strcmp(argv[2], "limit") == 0){
    if(argc < 4){
      usage();
      exit(1);
    }
    int limit = atoi(argv[3]);
    if(limit < 0){
      fprintf(2, "powerctl: limit must be >= 0\n");
      exit(1);
    }
    if(setcpulimit(pid, limit) < 0){
      fprintf(2, "powerctl: setcpulimit failed for pid %d\n", pid);
      exit(1);
    }
    printf("pid %d cpu limit set to %d ticks/window\n", pid, limit);
    exit(0);
  }

  if(strcmp(argv[2], "stats") == 0){
    int ticks = getcputicks(pid);
    if(ticks < 0){
      fprintf(2, "powerctl: failed to query stats for pid %d\n", pid);
      exit(1);
    }
    printf("pid %d cpu ticks total=%d\n", pid, ticks);
    exit(0);
  }

  usage();
  exit(1);
}
