#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){

    int sem_num = running->syscall_args[0];     //prendo gli argomenti passati alla semOpen che sono salvati nella variabile running in globals.h la quale è
    int value = running->syscall_args[1];       //un puntatore a una struct PCB che contiene un array (syscall_args) con tutti i parametri passati alla syscall

    printf("Sono nella sem_open di %d\n", sem_num);

    int fd = running->last_sem_fd;              //sempre da running prendo il descrittore dell'ultimo semaforo aperto

    ListHead semaphores_used = semaphores_list; //prendo la lista dei semafori creati, lista aggiunta in disastrOS_globals.h

    Semaphore* sem = SemaphoreList_byId(&semaphores_used,sem_num);   //ritorna il semaforo con id sem_num, o 0 se non lo trova
    if (!sem){                                  //semaforo non trovato,significa che non è stato creato, lo faccio ora
        sem = Semaphore_alloc(sem_num, value);
        if (!sem){                              //se la creazione è fallita, setto come valore di ritorno l'errore opportuno e termino
          running->syscall_retvalue = DSOS_ESEMALLOC;
          return;
        }
        List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);   //inserisco il semaforo appena creato in coda alla lista dei semafori creati
    }

    ListHead semaphores_opened = running->sem_descriptors;  //prendo la lista dei descrittori di semafori aperti
    SemDescriptor* already_opened = SemDescriptorList_byId(&semaphores_opened, sem_num); //controllo se il semaforo è stato già aperto, in tal caso ritorno il suo fd
    if (already_opened){
      running->syscall_retvalue = already_opened->fd;
      return;
    }


    SemDescriptor* sem_dsc = SemDescriptor_alloc(fd, sem, running);   //creo il sem_dsc per il semaforo
    if(!sem_dsc) {
        running->syscall_retvalue = DSOS_ESEMDSCALLOC;    //se la creazione è fallita, setto come valore di ritorno l'errore opportuno e termino
        return;
    }
    (running->last_sem_fd)+=1;                  //devo aggiornare il numero di sem_dsc aperti

    SemDescriptorPtr* sem_dsc_ptr = SemDescriptorPtr_alloc(sem_dsc);    //creo il puntatore alla entry nella lista delle risorse
    if(!sem_dsc_ptr) {
        running->syscall_retvalue = DSOS_ESEMDSCPTRALLOC;    //se la creazione è fallita, setto come valore di ritorno l'errore opportuno e termino
        return;
    }
    sem_dsc->ptr = sem_dsc_ptr;


    List_insert(&running-> sem_descriptors,running->sem_descriptors.last,(ListItem*) sem_dsc);  //aggiungo il sem_dsc alla lista di running


    List_insert(&sem -> descriptors, sem-> descriptors.last, (ListItem*) sem_dsc_ptr);      //aggiungo il sem_dsc_ptr alla lista del semaforo


    running -> syscall_retvalue = sem_dsc -> fd;    //il valore di ritorno dovrà essere il fd del semaforo



}
