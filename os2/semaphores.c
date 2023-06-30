#include "buffer_pool.h"

int create_semaphores(key_t key) {
    int semid = semget(key, NUM_BUFFERS, IPC_CREAT | 0666);
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