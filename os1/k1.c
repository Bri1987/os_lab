#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

/*
 * POSIX信号量实现
 */

#define N 5 // 哲学家的数量
#define LEFT (i)%N // 左侧筷子的编号
#define RIGHT (i+1)%N // 右侧筷子的编号

sem_t chopsticks[N]; // 筷子的互斥锁

void *philosopher(void *arg) {
    int i = *(int *)arg;
    while (1) {
        printf("Philosopher %d is thinking...\n", i);
        usleep(500); // 随机思考一段时间

        printf("Philosopher %d is hungry...\n", i);

        // 尝试拿起左侧的筷子
        //sem_trywait:如果信号量值大于0，则减1；等于0，则表示资源不可用
        int res1=sem_trywait(&chopsticks[LEFT]);
        if(res1<0)    //不成功
        {
            printf("Philosopher %d failed to pick up left chopstick %d.\n", i, LEFT);
            usleep(200);
            continue; // 如果无法拿起筷子，就让权等待一段时间
        }
        printf("Philosopher %d pick up left chopstick %d.\n", i, LEFT);

        int res2=sem_trywait(&chopsticks[RIGHT]);
        if(res2<0)    //不成功
        {
            printf("Philosopher %d failed to pick up right chopstick %d.\n", i, LEFT);
            //释放一下左筷子
            sem_post(&chopsticks[LEFT]);
            usleep(200);
            continue; // 如果无法拿起筷子，就让权等待一段时间
        }
        printf("Philosopher %d pick up right chopstick %d.\n", i, LEFT);

        printf("Philosopher %d is eating...\n", i);
        usleep(1000); // 随机吃饭一段时间

        sem_post(&chopsticks[LEFT]); // 放下左侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, LEFT);

        sem_post(&chopsticks[RIGHT]); // 放下右侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, RIGHT);
    }
}

int main() {
    pthread_t tid[N];
    int i, args[N];

    // 初始化POSIX信号量
    for (i = 0; i < N; i++) {
        sem_init(&chopsticks[i],0,1);     //第二个0代表是线程间共享的
    }

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
    for (i = 0; i < N; i++) {
        sem_destroy(&chopsticks[i]);
    }

    return 0;
}