#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"


void initFunction(void* args) {
    disastrOS_printStatus();
    printf("hello, I am init and I just started\n");
    int sem_num1 = 1;
    int sem_num2 = 2;

    //disastrOS_printStatus();
    int fd1 = disastrOS_semOpen(sem_num1, 0);
    disastrOS_printStatus();
    int fd2 = disastrOS_semOpen(sem_num2, 0);
    printf("%d -- %d\n", fd1, fd2);


    disastrOS_printStatus();
    printf("shutdown!\n");
    disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
