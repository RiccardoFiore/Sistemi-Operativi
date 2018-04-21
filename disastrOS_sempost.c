#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
    int fd= running->syscall_args[0];                   //prendo il fd del semaforo del processo corrente

    ListHead semaphores = running->sem_descriptors;     //prendo la lista dei semafori attivi del processo corrente

    SemDescriptor* sem_dsc = (SemDescriptor*) SemDescriptorList_byFd(&semaphores, fd);
    if(!sem_dsc){                                       //se il sem_dsc non esiste setto come valore di ritorno l'errore opportuno e termino
        running->syscall_retvalue = DSOS_ENOTMYSEM;
        return;
    }

    Semaphore* sem = sem_dsc->semaphore;                //prendo il semaforo dal sem_dsc

    if (!sem){                                          //il semaforo non è stato aperto correttamente, termino
        running->syscall_retvalue = DSOS_ESEMNOTOPENED;
        return;
    }

    (sem->count)++;

    if(sem->count <= 0){
        //prendo il processo in testa alla coda di attesa
        SemDescriptorPtr* head_wait_descriptor = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors), (ListItem*) (sem->waiting_descriptors).first);

        PCB* pcb_head = head_wait_descriptor->descriptor->pcb;              //prendo il pcb del processo appena preso

        List_detach(&waiting_list, (ListItem*) pcb_head);                               //lo rimuovo dalla lista di attesa...
        List_insert(&ready_list, (ListItem*) ready_list.last, (ListItem*) pcb_head);    //...e lo inserisco in quello dei processi ready

        pcb_head->status = Ready;                                           //cambio lo status del processo
    }

    running->syscall_retvalue = 0;                      //setto il valore di ritorno a 0, tutto ok

}
