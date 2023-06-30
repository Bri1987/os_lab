//
// Created by 12167 on 2023-04-26.
//

#ifndef OS2_3_SHARED_MEMORY_H
#define OS2_3_SHARED_MEMORY_H

//
// Created by 12167 on 2023-04-25.
//

#ifndef OS2_2_SHARED_MEMORY_H
#define OS2_2_SHARED_MEMORY_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define SHM_KEY 0x1234

int create_shared_memory(size_t size) {
    int shmid = shmget(SHM_KEY, size, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    return shmid;
}

void delete_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}

#endif //OS2_2_SHARED_MEMORY_H



#endif //OS2_3_SHARED_MEMORY_H
