#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);
    BufferPool *buffer_pool = (BufferPool *)shmat(shmid, NULL, 0);
    if (buffer_pool == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    for (int k = 0; k < 6; k++) {
        ///wait(full)
        // 消费者等待生产者通知,有通知才看，没通知就不看了
        // 获取当前时间和超时时间
        time_t start_time = time(NULL);
        time_t timeout = start_time + 5; // 5 秒钟超时

        // 等待生产者通知，超时则给出提示信息并让用户选择
        struct sembuf sops[1] = {{PRODUCER_NOTIFY, -1, IPC_NOWAIT}};
        while (semop(semid, sops, 1) == -1) {
            if (time(NULL) >= timeout) {
                printf("Consumer %d waiting timeout, press 'q' to quit or any others to wait longer...", getpid());
                char c ;
                scanf("%c",&c);
                if (c == 'q') {
                    shmdt(buffer_pool);
                    return 0;
                }
                //再等5秒
                timeout = time(NULL) + 5;
            }
        }

        ///wait(mutex)
        int record=buffer_pool->out;
        sops[0].sem_num = buffer_pool->out; // 获取互斥信号量
        sops[0].sem_op = -1;
        semop(semid, sops, 1);

        // 读取缓存的内容
        printf("Consumer %d read: %s in buffer %d\n", getpid(), buffer_pool->buffers[buffer_pool->out].buffer,record);

        // 更新out指针
        buffer_pool->out = (buffer_pool->out + 1) % NUM_BUFFERS;

        ///signal(mutex)
        sops[0].sem_num = record; // 释放互斥信号量
        sops[0].sem_op = 1;
        semop(semid, sops, 1);

        ///signal(empty)
        // 通知生产者缓冲区已经被消费
        sops[0].sem_num = CONSUMER_NOTIFY;
        sops[0].sem_op = 1;
        semop(semid, sops, 1);

        usleep(1000000);
    }

    shmdt(buffer_pool);
    return 0;
}