#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);
    BufferPool *buffer_pool = (BufferPool *)shmat(shmid, NULL, 0);
    if (buffer_pool == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    char content[BUFFER_SIZE];
    for (int k = 0; k < 4; k++) {
        // 先读取一行内容
        sprintf(content, "Producer %d:%d", getpid(), rand());

        ///wait(empty)
        // 生产者等待消费者通知,没通知不干了
        struct sembuf sops[1] = {{CONSUMER_NOTIFY, -1, 0}};
        semop(semid, sops, 1);

        ///wait(mutex)
        int record=buffer_pool->in;
        sops[0].sem_num = buffer_pool->in; // 获取互斥信号量
        sops[0].sem_op = -1;
        semop(semid, sops, 1);

        // 将内容写入缓存
        strncpy(buffer_pool->buffers[buffer_pool->in].buffer, content, BUFFER_SIZE);
        printf("%s in buffer %d\n",content,record);

        // 更新in指针
        buffer_pool->in = (buffer_pool->in + 1) % NUM_BUFFERS;

        ///signal(mutex)
        sops[0].sem_num = record; // 释放互斥信号量
        sops[0].sem_op = 1;
        semop(semid, sops, 1);

        ///signal(full)
        // 通知消费者缓冲区已经被生产
        sops[0].sem_num = PRODUCER_NOTIFY;
        sops[0].sem_op = 1;
        semop(semid, sops, 1);

        usleep(500000);
    }

    shmdt(buffer_pool);
    return 0;
}