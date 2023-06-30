#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"

//第一个参数是共享内存id，第二个参数是信号量
int main(int argc, char *argv[]) {
    int shmid=atoi(argv[1]);
    int semid=atoi(argv[2]);
    struct BufferPool *shm = (struct BufferPool *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    char*content= malloc((100));
    for(int k=0;k<100;k++) {
        //先读取一行内容
        sprintf(content,"Producer %d:%d",getpid(),rand());

        for (int j = 0; j < NUM_BUFFERS; ++j) {
            //信号量P操作
            struct sembuf sops[1] = {{j, -1, 0}};
            if (semop(semid, sops, 1) != -1) {
                //如果该内存还没有被写入
                if (shm->Index[j] == 0) {
                    //开写!
                    strncpy(shm->Buffer[j], content,BUFFER_SIZE);
                    printf("%s,is buffer %d\n", shm->Buffer[j],j);
                    shm->Index[j] = 1;

                    //sleep(1);
                    printf("producer %d out\n",getpid());

                    sops[0].sem_op = 1;      //释放资源
                    semop(semid, sops, 1);

                    //sleep(1);
                    break;
                }

                //释放锁资源
                sops[0].sem_op = 1;
                semop(semid, sops, 1);
            }
        }
        usleep(50000);
    }

    shmdt(shm);
}