//
// Created by 12167 on 2023-04-25.
//

#ifndef OS2_2_BUFFER_POOL_H
#define OS2_2_BUFFER_POOL_H

#define NUM_BUFFERS 10
#define BUFFER_SIZE 1024

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

//0-9的semid是互斥锁
#define PRODUCER_NOTIFY 10 // 生产者通知信号量
#define  CONSUMER_NOTIFY 11// 消费者通知信号量

#define NUM_BUFFERS 10
#define BUFFER_SIZE 1024

typedef struct {
    char buffer[BUFFER_SIZE];
} Buffer;

typedef struct {
    Buffer buffers[NUM_BUFFERS];
    int in;  // 生产者写入缓存的位置
    int out; // 消费者读取缓存的位置
} BufferPool;

#endif //OS2_2_BUFFER_POOL_H
