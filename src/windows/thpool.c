#include "thpool.h"
#include "threadpoolapiset.h"
#include "logger.h"
/*===============静态函数原型=============*/
typedef struct {
    void (*func)(void *);
    void *arg;
} ThreadData;
static void my_work_callback(PTP_CALLBACK_INSTANCE Instance,PVOID Context,PTP_WORK Work){
    (void)Instance;
    (void)Work;
    // 将上下文数据指针转换为所需类型
    ThreadData* data = (ThreadData*)Context;
    // 调用原始函数
    data->func(data->arg);
    // 释放上下文数据内存
    free(data);
}
struct thread_pool_t{
    PTP_POOL pool;// 线程池指针
};

// 线程池的初始化
thread_pool thpool_create(int thread_num){
    thread_pool pool = (thread_pool)malloc(sizeof(struct thread_pool_t));
    pool->pool = CreateThreadpool(NULL);
    SetThreadpoolThreadMaximum((PTP_POOL)pool->pool, thread_num);
    return pool;
}

// 向线程池中添加任务
int thpool_add_work(thread_pool pool, void (*func)(void *), void *arg){
    (void)pool;
    ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
    data->func = func;
    data->arg = arg;
    PTP_WORK work = CreateThreadpoolWork(my_work_callback, data, NULL);
    SubmitThreadpoolWork(work);
    return 0;
}

// 等待线程池中所有任务结束(Windows中不需要)
void thpool_wait(thread_pool pool){
    LOG_INFO("thpool_wait");
    (void)pool;
    //WaitForThreadpoolWorkCallbacks(pool->pool, FALSE);
}

// 销毁线程池
void thpool_destroy(thread_pool pool){
    LOG_INFO("thpool_destroy");
    CloseThreadpool(pool->pool);
    free(pool);
}

int thpool_is_start(thread_pool pool){
    return pool->pool != NULL;
}