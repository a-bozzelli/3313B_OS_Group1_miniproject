#include "kernel/types.h"
#include "user/user.h"

static void
burn_cpu(char *label)
{
  volatile uint x = 0;
  int pid = getpid();

  while(1){
    x++;
    if((x % 50000000) == 0){
      // Keep occasional output so demo shows the process is alive.
      printf("%s pid=%d cputicks=%d\n", label, pid, getcputicks(pid));
    }
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

  for(int i = 1; i <= 6; i++){
    pause(20);
    printf("sample %d: normal_ticks=%d eco_ticks=%d\n",
           i, getcputicks(normal), getcputicks(eco));
  }

  kill(normal);
  kill(eco);
  wait(0);
  wait(0);
  exit(0);
}
