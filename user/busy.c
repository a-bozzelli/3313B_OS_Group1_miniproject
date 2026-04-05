#include "user.h"

int
main(int argc, char **argv)
{
  printf("busy process started (pid = %d)\n", getpid());
  volatile unsigned long i = 0;
  while(1){
    i++;
    if(i % 10000000 == 0)
      ;
  }
  return 0;
}
