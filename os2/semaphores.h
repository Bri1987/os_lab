//
// Created by 12167 on 2023-04-18.
//

#ifndef OS2_SEMAPHORES_H
#define OS2_SEMAPHORES_H

#include <sys/types.h>

void delete_semaphores(int semid);
int create_semaphores(key_t key);

#endif //OS2_SEMAPHORES_H
