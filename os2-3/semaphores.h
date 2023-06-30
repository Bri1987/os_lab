//
// Created by 12167 on 2023-04-26.
//

#ifndef OS2_3_SEMAPHORES_H
#define OS2_3_SEMAPHORES_H

//
// Created by 12167 on 2023-04-25.
//

#ifndef OS2_2_SEMAPHORES_H
#define OS2_2_SEMAPHORES_H

#include <sys/sem.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int create_semaphores(key_t key, int num_sems) {
    int semid = semget(key, num_sems, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    return semid;
}

void delete_semaphores(int semid) {
    if (semctl(semid, 0, IPC_RMID, NULL) == -1) {
        perror("semctl");
        exit(1);
    }
}
#endif //OS2_2_SEMAPHORES_H


#endif //OS2_3_SEMAPHORES_H
