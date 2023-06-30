#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"

int main(int argc, char *argv[]) {
    int shmid=atoi(argv[1]);
    int semid=atoi(argv[2]);
    struct BufferPool *shm = (struct BufferPool *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    for(int k=0;k<100;k++){
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            struct sembuf sops[1] = {{i, -1, 0}};
            //加锁，不许其他访问
            if (semop(semid, sops, 1) != -1) {
                if (shm->Index[i] == 1) {
                    printf("Consumer %d: %s,in buffer %d\n", getpid(), shm->Buffer[i],i);
                    shm->Index[i] = 0;

                    //sleep(1);
                    printf("consumer %d out\n",getpid());
                    sops[0].sem_op = 1;
                    semop(semid, sops, 1);
                    break;
                }
                //释放锁
                sops[0].sem_op = 1;
                semop(semid, sops, 1);
            }
        }
        usleep(100000);
    }

    shmdt(shm);
}