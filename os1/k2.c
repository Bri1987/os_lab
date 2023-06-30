#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*
 * XSI信号量集
 */

#define N 5 // 哲学家的数量
#define LEFT (i)%N // 左侧筷子的编号
#define RIGHT (i+1)%N // 右侧筷子的编号
#define MYKEY 0x1a0a

int semid;    //一个信号量集的唯一标识符

//初始化信号量集
void init_semid(int init_value)
{
    //与控制命令配合的参数
    union semun{
        int val;
        struct semid_ds *buf;
        unsigned short *arry;
    };

    union semun sem_union;
    sem_union.val=init_value;
    //setVal信号量集操作
    for(int j=0;j<N;j++)
    {
        semctl(semid,j,SETVAL,sem_union);
    }
}

//P操作,同时操作两个信号量
//sembuf 是一个结构体类型，用于表示对一个 XSI 信号量集中的某个信号量进行操作时的参数
void sem_p(int sem_num)
{
    struct sembuf sem_buf[2];
    sem_buf[0].sem_num=sem_num;   //指示本次是操作信号量集的第sem_num个信号量
    sem_buf[0].sem_op=-1;   //-1代表是P操作
    sem_buf[0].sem_flg=SEM_UNDO;

    sem_buf[1].sem_num=(sem_num+1)%N;   //指示本次是操作信号量集的第sem_num个信号量
    sem_buf[1].sem_op=-1;   //-1代表是P操作
    sem_buf[1].sem_flg=SEM_UNDO;

    semop(semid,sem_buf,2);   //1代表第二个参数中sembuf结构数组的元素个数
}

//V操作，同时操作两个信号量
void sem_v(int sem_num)
{
    struct sembuf sem_buf[2];
    sem_buf[0].sem_num=sem_num;   //指示本次是操作信号量集的第sem_num个信号量
    sem_buf[0].sem_op=1;   //1代表是V操作
    sem_buf[0].sem_flg=SEM_UNDO;

    sem_buf[1].sem_num=(sem_num+1)%N;   //指示本次是操作信号量集的第sem_num个信号量
    sem_buf[1].sem_op=1;   //1代表是V操作
    sem_buf[1].sem_flg=SEM_UNDO;

    semop(semid,sem_buf,2);   //1代表第二个参数中sembuf结构数组的元素个数
}

void *philosopher(void *arg) {
    int i = *(int *)arg;
    while (1) {
        printf("Philosopher %d is thinking...\n", i);
        usleep(500); // 随机思考一段时间

        printf("Philosopher %d is hungry...\n", i);

        // 尝试拿起筷子
        sem_p(LEFT);
        printf("Philosopher %d is eating...\n", i);
        usleep(1000); // 吃饭一段时间

        //放下筷子
        sem_v(LEFT);
        printf("Philosopher %d finish eating...\n", i);
    }
}

int main() {
    pthread_t tid[N];
    int i, args[N];

    //创建XSI信号量集
    semid=semget(MYKEY,N,IPC_CREAT|0666);
    //初始化信号量集
    init_semid(1);

    // 创建哲学家线程
    for (i = 0; i < N; i++) {
        args[i] = i;
        pthread_create(&tid[i], NULL, philosopher, &args[i]);
    }

    // 等待哲学家线程结束
    for (i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }

    // 销毁POSIX信号量
    semctl(semid,0,IPC_RMID,NULL);

    return 0;
}