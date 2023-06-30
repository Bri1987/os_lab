#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_BUFFERS 10
#define BUFFER_SIZE 100

struct BufferPool {
    char Buffer[NUM_BUFFERS][BUFFER_SIZE];
    int Index[NUM_BUFFERS];
};

#endif // BUFFER_POOL_H