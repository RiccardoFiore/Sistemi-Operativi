#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "disastrOS.h"

#define MESSAGE_ERROR(cond, msg) do {                   \
        if (cond) {                                     \
            printf("%s: %d \n", msg,running->pid);      \
            disastrOS_exit(disastrOS_getpid()+1);       \
        }                                               \
    } while(0)
