#include "thpool.h"
#include "threadpoolapiset.h"

struct thread_pool_t{
    TP_POOL *pool;// 线程池指针
};

// 线程池的初始化
thread_pool thpool_create(int thread_num){
    thread_pool pool = (thread_pool)malloc(sizeof(struct thread_pool_t));
    pool->pool = CreateThreadpool(NULL);
    SetThreadpoolThreadMaximum(pool->pool, thread_num);
    return pool;
}

// 向线程池中添加任务
int thpool_add_work(thread_pool pool, void (*func)(void *), void *arg){
    PTP_WORK work = CreateThreadpoolWork(func, arg, NULL);
    SubmitThreadpoolWork(work);
    return 0;
}

// 等待线程池中所有任务结束
void thpool_wait(thread_pool pool){
    WaitForThreadpoolWorkCallbacks(pool->pool, FALSE);
}