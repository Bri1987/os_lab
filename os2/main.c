#include "buffer_pool.h"
#include "shared_memory.h"
#include "semaphores.h"

int main() {
    key_t key = ftok("buffer_pool.h", 'R');
    int shmid = create_shared_memory(key);
    int semid = create_semaphores(key);

    // Initialize semaphores to 1
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        semctl(semid, i, SETVAL, 1);
    }

    char* arg[4];
    arg[0]= malloc(100);arg[1]= malloc(100);arg[2]= malloc(100);
    sprintf(arg[1],"%d",shmid);
    sprintf(arg[2],"%d",semid);
    arg[3]=NULL;
    for(int i=0;i<4;i++)
    {
        pid_t pid=fork();
        if(pid==0 && i%2==0)
        {
            strcpy(arg[0],"./producer");
            execvp("./producer",arg);
        }
        else if(pid==0)
        {
            strcpy(arg[0],"./consumer");
            execvp("./consumer",arg);
        }
//        else
//        {
//            // 父进程等待所有子进程结束
//            waitpid(pid, NULL, 0);
//        }
    }


    for(int i=0;i<4;i++)
        wait(NULL);

    delete_shared_memory(shmid);
    delete_semaphores(semid);

    return 0;
}