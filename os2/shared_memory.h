//
// Created by 12167 on 2023-04-18.
//

#ifndef OS2_SHARED_MEMORY_H
#define OS2_SHARED_MEMORY_H

#include <sys/types.h>

void delete_shared_memory(int shmid);
int create_shared_memory(key_t key);

#endif //OS2_SHARED_MEMORY_H
