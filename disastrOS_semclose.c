#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_constants.h"

void internal_semClose(){


    int fd = running->syscall_args[0]; //prendo il file descriptor passato alla SemClose salvato nella variabile running in
                                       //globals.h

    printf("Sto chiudendo il semaforo con fd: %d\n", fd);

    int ret;

    //prendo il semaphore descriptor
    SemDescriptor* sem_dsc= SemDescriptorList_byFd(&running->sem_descriptors, fd);
    if(!sem_dsc){
        running->syscall_retvalue = DSOS_ESEMNOTEXISTS; //non ci sono semafori con questo fd aperti nel processo
        return;
    }

    //prendo il semaforo corrispondente al sem_dsc
    Semaphore* sem = sem_dsc->semaphore;

    SemDescriptorPtr* sem_dsc_ptr = sem_dsc->ptr;

    //elimino il sem_dsc dalla lista dei descrittori del processo chiamante
    sem_dsc = (SemDescriptor*) List_detach(&(running->sem_descriptors), (ListItem*) sem_dsc);
    ret = SemDescriptor_free(sem_dsc);
    if(ret) {
        running->syscall_retvalue = ret;
        return;
    }

    //elimino il puntatore a sem_dsc dalla lista del semaforo
    sem_dsc_ptr = (SemDescriptorPtr*) List_detach(&(sem->descriptors), (ListItem*) sem_dsc_ptr);
    ret = SemDescriptorPtr_free (sem_dsc_ptr);
    if(ret) {
        running->syscall_retvalue = ret;
        return;
    }


    //eliminati tutti i descrittori, eseguo la free di sem
    if((sem->descriptors).size == 0){
        sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
        ret = Semaphore_free(sem);
        if(ret != 0) {
            running->syscall_retvalue = ret;
            return;
        }

    }

    running->syscall_retvalue = 0;


}
