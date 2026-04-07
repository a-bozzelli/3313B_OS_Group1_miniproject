#include "kernel/types.h"
#include "user/user.h"

static void
burn_cpu(char *label)
{
  volatile uint x = 0;
  (void)label;

  while(1){
    x++;
  }
}

int
main(void)
{
  int normal = fork();
  if(normal < 0){
    fprintf(2, "powerdemo: fork failed for normal process\n");
    exit(1);
  }
  if(normal == 0){
    burn_cpu("normal");
    exit(0);
  }

  int eco = fork();
  if(eco < 0){
    fprintf(2, "powerdemo: fork failed for eco process\n");
    kill(normal);
    wait(0);
    exit(1);
  }
  if(eco == 0){
    burn_cpu("eco");
    exit(0);
  }

  pause(5);
  if(setpowermode(eco, 1) < 0 || setcpulimit(eco, 3) < 0){
    fprintf(2, "powerdemo: failed to configure throttling for pid %d\n", eco);
    kill(normal);
    kill(eco);
    wait(0);
    wait(0);
    exit(1);
  }

  printf("powerdemo: normal pid=%d, eco pid=%d (eco + budget=3 ticks/window)\n",
         normal, eco);
  pause(60);
  printf("powerdemo result: normal_ticks=%d eco_ticks=%d\n",
         getcputicks(normal), getcputicks(eco));

  kill(normal);
  kill(eco);
  wait(0);
  wait(0);
  exit(0);
}
