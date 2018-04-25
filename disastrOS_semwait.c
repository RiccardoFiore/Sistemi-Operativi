#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){

    int fd = running->syscall_args[0];                                                      //prendo il fd del semaforo del processo corrente


    ListHead proc_sem = running->sem_descriptors;                                           //lista dei semafori attivi del processo

    SemDescriptor* sem_dsc = (SemDescriptor*) SemDescriptorList_byFd(&proc_sem, fd);        //prendo il descrittore del semaforo con l'fd passato come argomento

    if(!sem_dsc) {                                                                          //il processo non ha aperto correttamente il semaforo
        running->syscall_retvalue = DSOS_EGETSEMDSC;
        return;
    }

    Semaphore* sem = sem_dsc->semaphore;                                                    //prendo il semaforo dal descrittore

    if (!sem){                                                                              //controllo per vedere se si è aperto correttamente
        running->syscall_retvalue = DSOS_ESEMNOTOPENED;
        return;
    }


    sem -> count -= 1;                                                                      //decremento il semaforo

    if (sem -> count < 0){                                                                  //a seconda di count decido che fare: se è < 0 allora il processo deve essere messo in attesa
        SemDescriptorPtr* sem_dsc_wait_ptr = SemDescriptorPtr_alloc(sem_dsc);               //alloco un puntatore al sem_dsc per gestire l'inserimento nella lista dei processi in waiting
        if(!sem_dsc_wait_ptr) {
            running->syscall_retvalue = DSOS_ESEMDSCPTRALLOC;
            return;
        }
        List_insert(&(sem_dsc->semaphore->waiting_descriptors), sem->waiting_descriptors.last, (ListItem*) sem_dsc_wait_ptr);
        running->status = Waiting;                                                          //metto il processo in waiting e faccio partire il prossimo nella ready queue
        List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
        //printf("\n****\n****\n****\nSto mettendo in waiting un processo\n****\n****\n****\n");        //stampe di prova da eliminare
        if (ready_list.first){
           // printf("\n****\n****\n****\n...e ne sto risvegliando un altro\n****\n****\n****\n");
            running = (PCB*) List_detach(&ready_list, ready_list.first);                    //risveglio il processo in testa alla coda dei processi ready
        }
        else {
            printf ("ATTENZIONE, SONO IN DEADLOCK, TERMINO DISASTROS.\n");
            disastrOS_shutdown();
        }

    }
    running -> syscall_retvalue = 0;                                                        //in caso di successo, il valore di ritorno è 0
    return;
}
