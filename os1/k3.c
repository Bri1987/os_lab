#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * 奇偶
 */

#define N 5 // 哲学家的数量
#define LEFT (i)%N // 左侧筷子的编号
#define RIGHT (i+1)%N // 右侧筷子的编号

pthread_mutex_t chopsticks[N]; // 筷子的互斥锁

void *philosopher(void *arg) {
    int i = *(int *)arg;
    while (1) {
        printf("Philosopher %d is thinking...\n", i);
        usleep(500); // 随机思考一段时间

        printf("Philosopher %d is hungry...\n", i);

        // 奇数: 尝试拿起左侧的筷子
        if(i%2==1)
        {
            pthread_mutex_lock(&chopsticks[LEFT]);
            printf("Philosopher %d picked up left chopstick %d.\n", i, LEFT);

            // 尝试拿起右侧的筷子
            pthread_mutex_lock(&chopsticks[RIGHT]);
            printf("Philosopher %d picked up right chopstick %d.\n", i, RIGHT);
        }
        else
        {
            pthread_mutex_lock(&chopsticks[RIGHT]);
            printf("Philosopher %d picked up right chopstick %d.\n", i, RIGHT);

            // 尝试拿起左侧的筷子
            pthread_mutex_lock(&chopsticks[LEFT]);
            printf("Philosopher %d picked up left chopstick %d.\n", i, LEFT);
        }

        printf("Philosopher %d is eating...\n", i);
        usleep(1000); // 随机吃饭一段时间

        pthread_mutex_unlock(&chopsticks[LEFT]); // 放下左侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, LEFT);

        pthread_mutex_unlock(&chopsticks[RIGHT]); // 放下右侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, RIGHT);
    }
}

int main() {
    pthread_t tid[N];
    int i, args[N];

    // 初始化互斥锁
    for (i = 0; i < N; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
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

    // 销毁互斥锁
    for (i = 0; i < N; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}