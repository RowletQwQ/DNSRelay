//线程池相关函数声明
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifdef _WIN32
#include <windows.h>
#include <process.h>

typedef HANDLE thread_t;
typedef CRITICAL_SECTION mutex_t;
typedef CONDITION_VARIABLE cond_t;
typedef HANDLE my_sem_t;

#else
#include <pthread.h>
#include <semaphore.h>

typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
typedef sem_t my_sem_t;

#endif
//线程池
struct thread_pool_t{
    thread_t *threads; //线程数组
    int thread_count; //线程数量
    my_sem_t tasks; //任务信号量,表示任务数量
    my_sem_t spaces; //空闲线程信号量,表示空闲线程数量
    mutex_t tasks_mutex; //任务互斥锁
    cond_t tasks_cond; //任务条件变量
    int front; //任务队列头
    int rear; //任务队列尾
    int tasks_count; //任务数量
    int shutdown; //线程池是否关闭
    void (*process)(void *arg); //任务处理函数
    void **task_arg; //任务参数
};
typedef struct thread_pool_t thread_pool_t;

/**
 * @brief 创建线程池
 * 
 * @param pool 线程池指针
 * @param thread_count 线程数量
 * @param process 任务处理函数
 * @return int 0表示成功,其他表示失败
 */
int thread_pool_init(thread_pool_t *pool, int thread_count, void (*process)(void *arg));

/**
 * @brief 销毁线程池
 * 
 * @param pool 线程池指针
 * @return int 0表示成功,其他表示失败
 */
int thread_pool_destroy(thread_pool_t *pool);

/**
 * @brief 提交任务
 * 
 * @param pool 线程池指针
 * @param arg 任务参数
 * @return int 0表示成功,其他表示失败
 */
int thread_pool_submit(thread_pool_t *pool, void *arg);

/**
 * @brief 关闭线程池
 * 
 * @param pool 线程池指针
 * @return int 0表示成功,其他表示失败
 */
int thread_pool_shutdown(thread_pool_t *pool);

//互斥锁
/**
 * @brief 初始化互斥锁
 * 
 * @param mutex 互斥锁指针
 * @return int 0表示成功,其他表示失败
 */
int mutex_init(mutex_t *mutex);

/**
 * @brief 销毁互斥锁
 * 
 * @param mutex 互斥锁指针
 * @return int 0表示成功,其他表示失败
 */
int mutex_destroy(mutex_t *mutex);

/**
 * @brief 加锁
 * 
 * @param mutex 互斥锁指针
 * @return int 0表示成功,其他表示失败
 */
int mutex_lock(mutex_t *mutex);

/**
 * @brief 解锁
 * 
 * @param mutex 互斥锁指针
 * @return int 0表示成功,其他表示失败
 */
int mutex_unlock(mutex_t *mutex);

//条件变量
/**
 * @brief 初始化条件变量
 * 
 * @param cond 条件变量指针
 * @return int 0表示成功,其他表示失败
 */
int cond_init(cond_t *cond);

/**
 * @brief 销毁条件变量
 * 
 * @param cond 条件变量指针
 * @return int 0表示成功,其他表示失败
 */
int cond_destroy(cond_t *cond);

/**
 * @brief 等待条件变量
 * 
 * @param cond 条件变量指针
 * @param mutex 互斥锁指针
 * @return int 0表示成功,其他表示失败
 */
int cond_wait(cond_t *cond, mutex_t *mutex);

/**
 * @brief 唤醒条件变量
 * 
 * @param cond 条件变量指针
 * @return int 0表示成功,其他表示失败
 */
int cond_signal(cond_t *cond);

//信号量
/**
 * @brief 初始化信号量
 * 
 * @param sem 信号量指针
 * @param value 信号量初始值
 * @return int 0表示成功,其他表示失败
 */
int my_sem_init(my_sem_t *sem, int value);

/**
 * @brief 销毁信号量
 * 
 * @param sem 信号量指针
 * @return int 0表示成功,其他表示失败
 */
int my_sem_destroy(my_sem_t *sem);

/**
 * @brief 等待信号量
 * 
 * @param sem 信号量指针
 * @return int 0表示成功,其他表示失败
 */
int my_sem_wait(my_sem_t *sem);

/**
 * @brief 发送信号量
 * 
 * @param sem 信号量指针
 * @return int 0表示成功,其他表示失败
 */
int my_sem_post(my_sem_t *sem);

#endif