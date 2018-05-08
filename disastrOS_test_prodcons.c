#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "disastrOS.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_globals.h"
#include "common.h"

#define SEM_EMPTY 0
#define SEM_FILL 1
#define SEM_PROD 2
#define SEM_CONS 3
#define BUFFER_SIZE 4
#define CYCLES 10
#define N 8

int read_index,write_index, sum;                                    //indici di lettura, scrittura e somma dei "token" prodotti nelle operazioni
int op[BUFFER_SIZE];

void initFunction(void* args);
void prodFunction(void* args);
void consFunction(void* args);

// we need this to handle the sleep state
void sleeperFunction(void* args){
    printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
    while(1) {
        getc(stdin);
        disastrOS_printStatus();
    }
}


void prodFunction(void* args){
    int i,ret;

    printf("Sono il produttore con pid : %d\n",running->pid);
    int sem_fill= disastrOS_semOpen(SEM_FILL, 0);                       //inizializzo il semaforo a 0, poichè sono stati prodotti 0 "token"
    MESSAGE_ERROR(sem_fill < 0,"Errore nella semOpen di sem_fill del processp ");

    int sem_empty = disastrOS_semOpen(SEM_EMPTY, BUFFER_SIZE);          //inizializzo il semaforo a BUFFER_SIZE, poichè ho ancora tutto lo spazio a disposizione
    MESSAGE_ERROR(sem_empty < 0,"Errore nella semOpen di sem_empty del processo ");

    int sem_prod = disastrOS_semOpen(SEM_PROD, 1);                      //semaforo che gestisce le operazioni tra i processi Produttori
    MESSAGE_ERROR(sem_prod < 0,"Errore nella semOpen di sem_prod del processo ");

    ListHead semaphores_used = semaphores_list;
    Semaphore* sem_e = SemaphoreList_byId(&semaphores_used,sem_empty);
    Semaphore* sem_f = SemaphoreList_byId(&semaphores_used,sem_fill);

    for(i = 0;i < CYCLES;i++){
        printf("\n\n+++++\n+++++\n+++++\nPid: %d\nsem_empty: %d\nsem_fill: %d\n+++++\n+++++\n+++++\n\n",running->pid, sem_e->count, sem_f->count);
        ret = disastrOS_semWait(sem_empty);                             //devo aspettare che sem_empty abbia almeno uno spazio per poter inserire il "token"
        MESSAGE_ERROR(ret != 0, "Errore nella semWait di sem_empty del processo ");
        ret = disastrOS_semWait(sem_prod);                              //devo aspettare che sia il mio turno tra tutti i Produttori
        MESSAGE_ERROR(ret != 0, "Errore nella semWait di sem_prod del processo ");

        op[write_index] = running->pid;                       //produco il "token"
        write_index = (write_index + 1) % BUFFER_SIZE;

        ret = disastrOS_semPost(sem_prod);                              //comunico agli altri sem. Produttori che ho finito
        MESSAGE_ERROR(ret != 0, "Errore nella semPost di sem_prod del processo ");

        ret = disastrOS_semPost(sem_fill);                              //incremento sem_fill poichè ho inserito un "token"
        MESSAGE_ERROR(ret != 0, "Errore nella semPost di sem_fill del processo ");
    }

    ret = disastrOS_semClose(sem_fill);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_fill del processo ");

    ret = disastrOS_semClose(sem_empty);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_empty del processo ");

    ret = disastrOS_semClose(sem_prod);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_prod del processo");

    disastrOS_exit(disastrOS_getpid()+1);
}

void consFunction(void* args){
    int i,ret;

    printf("Sono il consumatore con pid : %d\n",running->pid);

    int sem_fill= disastrOS_semOpen(SEM_FILL, 0);
    MESSAGE_ERROR(sem_fill < 0,"Errore nella semOpen di sem_fill del processo ");

    int sem_empty = disastrOS_semOpen(SEM_EMPTY, BUFFER_SIZE);
    MESSAGE_ERROR(sem_empty < 0,"Errore nella semOpen di sem_empty del processo ");

    int sem_cons = disastrOS_semOpen(SEM_CONS, 1);
    MESSAGE_ERROR( sem_cons < 0,"Errore nella semOpen di sem_cons del processo ");

    ListHead semaphores_used = semaphores_list;
    Semaphore* sem_e = SemaphoreList_byId(&semaphores_used,sem_empty);
    Semaphore* sem_f = SemaphoreList_byId(&semaphores_used,sem_fill);

    for(i = 0;i < CYCLES;i++){
        printf("\n\n+++++\n+++++\n+++++\nPid: %d\nsem_empty: %d\nsem_fill: %d\n+++++\n+++++\n+++++\n\n",running->pid, sem_e->count, sem_f->count);
        ret = disastrOS_semWait(sem_fill);                                            //aspetto finchè sem_fill non contenga almeno un "token" prodotto
        MESSAGE_ERROR(ret != 0, "Errore nella semWait di sem_fill del processo ");

        ret = disastrOS_semWait(sem_cons);                                            //semaforo che regola le operazioni dei Consumatori
        MESSAGE_ERROR(ret != 0, "Errore nella semWait di sem_cons del processo ");

        int new_op = op[read_index];
        read_index = (read_index + 1) % BUFFER_SIZE;
        sum += new_op;
        if (read_index % N == 0) {
            printf("La somma totale delle operazioni è: %d.\n", sum);
        }

        ret = disastrOS_semPost(sem_cons);                                            //comunico agli altri Consumatori che ho finito
        MESSAGE_ERROR(ret != 0, "Errore nella semPost di sem_cons del processo ");

        ret = disastrOS_semPost(sem_empty);                                           //incremento sem_empty poichè ho liberato uno spazio
        MESSAGE_ERROR(ret != 0, "Errore nella semPost di sem_empty del processo ");
    }

    ret = disastrOS_semClose(sem_fill);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_fill del processo ");

    ret = disastrOS_semClose(sem_empty);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_empty del processo ");

    ret = disastrOS_semClose(sem_cons);
    MESSAGE_ERROR(ret != 0, "Errore nella semClose di sem_cons del processo ");

    disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
    disastrOS_printStatus();
    printf("Sono nel processo init con pid=%d\n",running->pid);
    disastrOS_spawn(sleeperFunction, 0);

    write_index=read_index=0;               //inizializzo write index e read_index

    int children=0;
    int i;
    int fd[N];

    for (i=0; i<N/2; ++i) {
        int type=0;
        int mode=DSOS_CREATE;
        printf("mode: %d\n", mode);
        printf("opening resource\n");
        fd[i]=disastrOS_openResource(i,type,mode);
        printf("fd=%d\n", fd[i]);
        disastrOS_spawn(prodFunction, 0);
        children++;
    }

    for (; i<N; ++i) {
        int type=0;
        int mode=DSOS_CREATE;
        printf("mode: %d\n", mode);
        printf("opening resource\n");
        fd[i]=disastrOS_openResource(i,type,mode);
        printf("fd=%d\n", fd[i]);
        disastrOS_spawn(consFunction, 0);
        children++;
    }
    int retval;
    int pid;
    while(children>0 && (pid=disastrOS_wait(0, &retval))>=0){
        printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
         pid, retval, children);
        --children;
    }
    for (i=0; i<N; ++i) {
        printf("closing resource %d\n",fd[i]);
        disastrOS_closeResource(fd[i]);
        disastrOS_destroyResource(i);
    }

    printf("SHUTDOWN!\n");
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

    printf("START\n");
    disastrOS_start(initFunction, 0, logfilename);
    return 0;
}
