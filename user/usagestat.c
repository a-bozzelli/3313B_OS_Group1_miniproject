#include "user.h"

int
main(int argc, char **argv)
{
  if(argc != 2){
    printf("usage: usagestat <pid>\n");
    return 1;
  }
  int pid = atoi(argv[1]);
  struct usagestats st;
  if(getusagestats(pid, &st) < 0){
    printf("Process not found\n");
    return 1;
  }
  printf("PID: %d | CPU Ticks: %lu | Limit: %lu\n",
         pid, (unsigned long)st.cpu_ticks, (unsigned long)st.cpu_ticks_limit);
  return 0;
}
