#include "user.h"

int
main(int argc, char **argv)
{
  if(argc != 3){
    printf("usage: usagectl <pid> <max_ticks>\n");
    return 1;
  }
  int pid = atoi(argv[1]);
  int max = atoi(argv[2]);
  if(setusagelimit(pid, max) < 0){
    printf("Error: invalid PID or failed to set limit\n");
    return 1;
  }
  printf("Limit set: PID %d → %d ticks\n", pid, max);
  return 0;
}
