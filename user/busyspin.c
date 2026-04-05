#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int reports = 10;
  char *label = "busyspin";
  int pid = getpid();

  if(argc > 1)
    label = argv[1];
  if(argc > 2){
    reports = atoi(argv[2]);
    if(reports < 0)
      reports = 0;
  }

  for(int r = 1; reports == 0 || r <= reports; r++){
    for(volatile int i = 0; i < 40000000; i++){
      // Intentional busy loop to consume CPU.
    }
    printf("%s pid=%d report=%d uptime=%d cputicks=%d\n",
           label, pid, r, uptime(), getcputicks(pid));
  }

  exit(0);
}
