#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * 安全检查
 */

#define N 10 // 哲学家的数量
#define LEFT (i)%N // 左侧筷子的编号
#define RIGHT (i+1)%N // 右侧筷子的编号

pthread_mutex_t chopsticks[N]; // 筷子的互斥锁
pthread_mutex_t safety;
int allocation[N]={0};

int flag_check= 0;

// 检查系统是否处于安全状态
//即判断是否死锁了
//allocation元素的取值为0,1,2，如果都是1，就是每个人都拿了左边筷子，就是死锁
//void * check_safety() {
//    for(int i=0;i<N;i++)
//    {
//        if(allocation[i]!=1)
//        {
//            flag_check= 1;
//            break;
//        }
//    }
//    printf("okkkkkk\n");
//    flag_check=0;
//    return NULL;
//}

void *philosopher(void *arg) {
    int i = *(int *)arg;
    while (1) {
        printf("Philosopher %d is thinking...\n", i);
        //usleep(500); // 随机思考一段时间

        printf("Philosopher %d is hungry...\n", i);

        pthread_mutex_lock(&chopsticks[LEFT]);

        pthread_mutex_lock(&safety);    //同一时间只能有一个在进行safety判断
        allocation[i]=1;

        for(int j=0;j<N;j++)
        {
            if(allocation[j]!=1)
            {
                flag_check= 1;
                break;
            }
        }

        if(flag_check==0)
            printf("ggg\n");
        if(flag_check==1)
        {
            flag_check=0;
            printf("Philosopher %d picked up left chopstick %d.\n", i, LEFT);
            pthread_mutex_unlock(&safety);
            // 尝试拿起右侧的筷子
            pthread_mutex_lock(&chopsticks[RIGHT]);
            allocation[i]=2;
            printf("Philosopher %d picked up right chopstick %d.\n", i, RIGHT);
        }
        else
        {
            //死锁了,赶紧释放掉左边的锁，并重新思考
            printf("Philosopher %d failed to pick up left chopstick %d.\n", i, LEFT);
            pthread_mutex_unlock(&chopsticks[LEFT]); // 放下左侧的筷子
            allocation[i]=0;
            pthread_mutex_unlock(&safety);
            usleep(200);
            continue;
        }

        printf("Philosopher %d is eating...\n", i);
        usleep(1000); // 随机吃饭一段时间

        pthread_mutex_unlock(&chopsticks[LEFT]); // 放下左侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, LEFT);

        pthread_mutex_unlock(&chopsticks[RIGHT]); // 放下右侧的筷子
        printf("Philosopher %d put down chopstick %d.\n", i, RIGHT);

        allocation[i]=0;
    }
}

int main() {
    pthread_t tid[N];
    int i, args[N];

    // 初始化互斥锁
    for (i = 0; i < N; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }
    pthread_mutex_init(&safety,NULL);


     //创建安全状态线程
//    pthread_t safety_thread;
//    pthread_create(&safety_thread, NULL, check_safety, NULL);

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