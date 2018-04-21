#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

#define NUM_PROC 10

void childFunction1(void* args){
    printf("Hello, I am the childfunction1 %d\n",disastrOS_getpid());

    int type=0;
    int mode=0;
    int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
    printf("PID: %d,fd: %d\n", disastrOS_getpid(), fd);
    int sem_num1 = 1;

    int fd1 = disastrOS_semOpen(sem_num1, NUM_PROC);

    int ret = disastrOS_semWait(fd1);
    if (ret) printf("Errore nella semWait del processo: %d\n", disastrOS_getpid());

    disastrOS_printStatus();
    printf("***\n***\n***\n Ho finito la semWait, termino\n***\n***\n***\n");
    disastrOS_exit(disastrOS_getpid()+1);
}

void childFunction2(void* args){
    printf("Hello, I am the childfunction2 %d\n",disastrOS_getpid());

    int type=0;
    int mode=0;
    int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
    printf("PID: %d,fd: %d\n", disastrOS_getpid(), fd);
    int sem_num2 = 2;

    int fd2 = disastrOS_semOpen(sem_num2, NUM_PROC-1);

    int ret = disastrOS_semWait(fd2);
    if (ret) printf("Errore nella semWait del processo: %d\n", disastrOS_getpid());

    disastrOS_printStatus();
    printf("***\n***\n***\n Ho finito la semWait, termino\n***\n***\n***\n");
    disastrOS_exit(disastrOS_getpid()+1);
}

void childFunction3(void* args){
    printf("Hello, I am the childfunction2 %d\n",disastrOS_getpid());

    int type=0;
    int mode=0;
    int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
    printf("PID: %d,fd: %d\n", disastrOS_getpid(), fd);
    int sem_num3 = 3;

    int fd3 = disastrOS_semOpen(sem_num3, 0);
    int ret = disastrOS_semPost(fd3);
    if (ret) printf("Errore nella semPost del processo: %d\n", disastrOS_getpid());

    disastrOS_printStatus();
    printf("***\n***\n***\n Ho finito la semPost, termino\n***\n***\n***\n");

    disastrOS_exit(disastrOS_getpid()+1);
}

void initFunction(void* args) {
    disastrOS_printStatus();
    printf("hello, I am init and I just started\n");

    printf("Inizio a lavorare con i semafori, li apro\n");
    int sem_num1 = 1;
    int sem_num2 = 2;
    int sem_num3 = 3;
    int fd1 = disastrOS_semOpen(sem_num1, NUM_PROC);
    int fd2 = disastrOS_semOpen(sem_num2, NUM_PROC-1);
    int fd3 = disastrOS_semOpen(sem_num3, 0);
    int ret;

    printf("I feel like to spawn 10 nice threads\n");
    int alive_children=0, i;
    for (i=0; i < NUM_PROC; ++i) {
        int type=0;
        int mode=DSOS_CREATE;
        printf("mode: %d\n", mode);
        printf("opening resource (and creating if necessary)\n");
        int fd=disastrOS_openResource(i,type,mode);
        printf("fd=%d\n", fd);
        disastrOS_spawn(childFunction1, 0);              //crea i figli che eseguono la funzione childFunction1
        disastrOS_spawn(childFunction3, 0);              //crea i figli che eseguono la funzione childFunction3 che testa la semPost
        disastrOS_spawn(childFunction2, 0);              //crea i figli che eseguono la funzione childFunction2,la quale deve causare deadlock
        alive_children += 3;
    }
    int retval;
    int pid;
    while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
        printf("initFunction, child: %d terminated, retval:%d, alive: %d \n", pid, retval, alive_children);
        --alive_children;
    }

    printf("Ho finito di lavorare con i semafori, li chiudo\n");
    ret=disastrOS_semClose(fd1);
    if(ret!=0) printf("Errore nella semClose del semaforo con fd %d\n",fd1);
    ret=disastrOS_semClose(fd2);
    if(ret!=0) printf("Errore nella semClose del semaforo con fd %d\n",fd2);
    ret=disastrOS_semClose(fd3);
    if(ret!=0) printf("Errore nella semClose del semaforo con fd %d\n",fd3);

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
