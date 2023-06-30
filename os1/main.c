#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/*
 * version1：会死锁
 * version2:互斥锁实现
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

//  version:1
//        pthread_mutex_lock(&chopsticks[LEFT]); // 拿起左侧的筷子
//        printf("Philosopher %d picked up chopstick %d.\n", i, LEFT);
//
//        pthread_mutex_lock(&chopsticks[RIGHT]); // 拿起右侧的筷子
//        printf("Philosopher %d picked up chopstick %d.\n", i, RIGHT);


//  version:2
        // 尝试拿起左侧的筷子
        int res1 = pthread_mutex_trylock(&chopsticks[LEFT]);
        //返回0表示加锁成功
        if (res1 != 0) {
            printf("Philosopher %d failed to pick up left chopstick %d.\n", i, LEFT);
            usleep(200);
            continue; // 如果无法拿起筷子，就让权等待一段时间
        }
        printf("Philosopher %d picked up left chopstick %d.\n", i, LEFT);

        // 尝试拿起右侧的筷子
        int res2 = pthread_mutex_trylock(&chopsticks[RIGHT]);
        if (res2 != 0) {
            pthread_mutex_unlock(&chopsticks[LEFT]); // 要放下左侧的筷子
            printf("Philosopher %d failed to pick up right chopstick %d.\n", i, RIGHT);
            usleep(200);
            continue; // 如果无法拿起筷子，就让权等待一段时间
        }
        printf("Philosopher %d picked up right chopstick %d.\n", i, RIGHT);

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