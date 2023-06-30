#include <string.h>
#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"

int main() {
    size_t size=8192;
    key_t key2=0x12;
    int shmid = create_shared_memory(size);
    int semid = create_semaphores(key2, 3+NUM_BUFFERS); // 三个信号量

    // Initialize semaphores to 1
    // Initialize semaphores to 1
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        semctl(semid, i, SETVAL, 1);
    }

    semctl(semid, PRODUCER_NOTIFY, SETVAL, 0); // 生产者通知信号量初始为0
    semctl(semid, CONSUMER_NOTIFY, SETVAL, NUM_BUFFERS); // 消费者通知信号量初始

    // 将共享内存附加到进程地址空间
    BufferPool *buffer_pool = (BufferPool *)shmat(shmid, NULL, 0);
    if (buffer_pool == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    // 初始化缓存池
    buffer_pool->in = 0;
    buffer_pool->out = 0;

    char* arg[4];
    arg[0] = malloc(100); arg[1] = malloc(100); arg[2] = malloc(100);
    sprintf(arg[1],"%d",shmid);
    sprintf(arg[2],"%d",semid);
    arg[3] = NULL;
    for(int i=0; i<4; i++) {
        pid_t pid = fork();
        if(pid == 0 && i%2 == 0) {
            strcpy(arg[0],"./producer");
            execvp("./producer",arg);
        }
        else if(pid == 0) {
            strcpy(arg[0],"./consumer");
            execvp("./consumer",arg);
        }
    }

    for(int i=0;i<4;i++)
        wait(NULL);

    delete_shared_memory(shmid);
    delete_semaphores(semid);

    return 0;
}