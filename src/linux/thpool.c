#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#include "datatype.h"
#include "linked_list.h"
#include "thpool.h"
#include "logger.h"


//Linux下的线程池实现
static volatile int threads_keepalive;// 线程池中线程的最长空闲时间
static volatile int threads_on_hold;// 线程池中线程的数量
/*============================== 结构体 =============================*/
/* 信号量 */
typedef struct semaphore {
    pthread_mutex_t mutex;// 互斥锁,实现二元信号量的互斥访问
    pthread_cond_t cond;// 条件变量,实现二元信号量的等待和唤醒操作
    int v;// 整数变量,表示当前信号量的值
} semaphore_t;

/* 工作 */
typedef struct work {
    struct work *prev; // 指向上一个任务
    void (*function)(void *arg); // 函数指针,指向任务函数
    void *arg; // 任务函数的参数
} work_t;
/* 工作队列,基于双向链表实现 */

typedef struct work_queue {
    pthread_mutex_t rwmutex; // 读写锁
    list_ops_unit_t list; // 双向链表
    semaphore_t *has_jobs; // 信号量
    int size; // 队列中任务的数量
} work_queue_t;

typedef struct thread_pool_t thread_pool_t;
/* 工作线程 */
typedef struct work_thread {
    int id; // 线程id
    pthread_t thread; // 线程
    thread_pool_t* thread_pool_ptr; // 对应的线程池
} thread_t;
/* 线程池 */
typedef struct thread_pool_t {
    thread_t** threads; // 线程池中线程的数组
    int num_thread_alive; // 线程池中存活的线程数量
    int num_thread_working; // 线程池中正在工作的线程数量
    pthread_mutex_t thcount_lock; // 互斥锁,在线程技术等时候使用
    pthread_cond_t threads_all_idle; // 条件变量,thpool_wait()函数中使用
    work_queue_t work_queue; // 工作队列
} thread_pool_t;

/*====================内部函数原型====================*/

static int thread_init(thread_t **thread_ptr, thread_pool_t * thread_pool_ptr, int id);
static void *thread_do(thread_t *thread);
static void thread_hold(int sig_id);
static void thread_destroy(thread_t *thread);

static int work_queue_init(work_queue_t *work_queue);
static void work_queue_clear(work_queue_t *work_queue);
static void work_queue_push(work_queue_t *work_queue, work_t *work);
static work_t *work_queue_pull(work_queue_t *work_queue);
static void work_queue_destroy(work_queue_t *work_queue);

static void semaphore_init(semaphore_t *semaphore, int init_value);
static void semaphore_wait(semaphore_t *semaphore);
static void semaphore_post(semaphore_t *semaphore);
static void semaphore_reset(semaphore_t *semaphore);
static void semaphore_post_all(semaphore_t *semaphore);

/*=====================线程池相关=====================*/


/* 创建线程池 */
thread_pool_t* thpool_create(int thread_num){
    threads_on_hold = 0;
    threads_keepalive = 1;

    if (thread_num < 0) {
        thread_num = 0;
    }
    // 新建线程池
    thread_pool_t *thread_pool_ptr = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (thread_pool_ptr == NULL) {
        LOG_ERROR("thpool_create(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    thread_pool_ptr->num_thread_alive = 0;
    thread_pool_ptr->num_thread_working = 0;

    // 初始化工作队列
    if(work_queue_init(&thread_pool_ptr->work_queue) == THPOOL_FAIL){
        LOG_ERROR("thpool_create(): Could not allocate memory for work queue\n");
        free(thread_pool_ptr);
        return NULL;
    }

    // 初始化池中的线程
    thread_pool_ptr->threads = (thread_t **)malloc(sizeof(thread_t *) * thread_num);
    if (thread_pool_ptr->threads == NULL) {
        LOG_ERROR("thpool_create(): Could not allocate memory for threads\n");
        work_queue_destroy(&thread_pool_ptr->work_queue);
        free(thread_pool_ptr);
        return NULL;
    }

    pthread_mutex_init(&(thread_pool_ptr->thcount_lock), NULL);
    pthread_cond_init(&(thread_pool_ptr->threads_all_idle), NULL);

    // 线程初始化
    for (int i = 0; i < thread_num; i++) {
        thread_init(&(thread_pool_ptr->threads[i]), thread_pool_ptr, i);
        LOG_DEBUG("thpool_create(): Created thread %d in pool \n", i);
    }

    // 等待所有线程启动
    while (thread_pool_ptr->num_thread_alive != thread_num) {}
    return thread_pool_ptr;
}

// 把任务添加到线程池中
int thpool_add_work(thread_pool_t *thread_pool_ptr, void (*func)(void *), void *arg){
    LOG_DEBUG("thpool_add_work(): add work to thread pool\n");
    work_t *work;
    work = (work_t *)malloc(sizeof(work_t));
    if (work == NULL) {
        LOG_ERROR("thpool_add_work(): Could not allocate memory for new work\n");
        return THPOOL_FAIL;
    }
    work->function = func;
    work->arg = arg;

    work_queue_push(&thread_pool_ptr->work_queue, work);
    LOG_DEBUG("thpool_add_work(): add work to work queue success\n");
    return THPOOL_SUCCESS;
}

// 等待线程池中的所有线程完成任务
void thpool_wait(thread_pool_t *thread_pool_ptr){
    LOG_DEBUG("thpool_wait(): wait for all threads to finish\n");
    pthread_mutex_lock(&thread_pool_ptr->thcount_lock);
    while (thread_pool_ptr->work_queue.size || thread_pool_ptr->num_thread_working) {
        pthread_cond_wait(&thread_pool_ptr->threads_all_idle, &thread_pool_ptr->thcount_lock);
    }
    pthread_mutex_unlock(&thread_pool_ptr->thcount_lock);
    LOG_DEBUG("thpool_wait(): all threads finish\n");
}

// 销毁线程池
void thpool_destroy(thread_pool_t *thread_pool_ptr){
    // 如果线程池已经销毁,则直接返回
    if (thread_pool_ptr == NULL || thread_pool_ptr->num_thread_alive == 0) {
        return;
    }
    volatile int threads_total = thread_pool_ptr->num_thread_alive;

    // 通知所有线程退出
    threads_keepalive = 0;
    // 唤醒所有线程
    semaphore_post_all(thread_pool_ptr->work_queue.has_jobs);
    for (int i = 0; i < threads_total; i++) {
        pthread_join(thread_pool_ptr->threads[i]->thread, NULL);
    }

    // 清除工作队列
    work_queue_destroy(&thread_pool_ptr->work_queue);

    // 释放线程池
    for (int i = 0; i < threads_total; i++) {
        thread_destroy(thread_pool_ptr->threads[i]);
    }
    free(thread_pool_ptr->threads);
    free(thread_pool_ptr);
}

// 暂停线程池中的所有线程
void thpool_pause(thread_pool_t *thread_pool_ptr){
    for (int i = 0; i < thread_pool_ptr->num_thread_alive; i++) {
        pthread_kill(thread_pool_ptr->threads[i]->thread, SIGUSR1);
    }
}

// 恢复线程池中的所有线程
void thpool_resume(thread_pool_t *thread_pool_ptr){
    //TODO 增加单一线程池恢复的支持
    (void)thread_pool_ptr;
    threads_on_hold = 0;
}

// 确认线程池是否开启
int thpool_is_start(thread_pool_t *thread_pool_ptr){
    if(thread_pool_ptr == NULL){
        return 0;
    }
    return thread_pool_ptr->num_thread_alive == thread_pool_ptr->num_thread_working;
}

/*=====================线程相关=====================*/

/**
 * @brief 从线程池中初始化一个线程
 * 
 * @param thread_ptr 指向线程的指针
 * @param thread_pool_ptr 指向线程池的指针
 * @param id 线程id
 * @return int 成功返回THPOOL_SUCCESS,失败返回THPOOL_FAIL
 */
static int thread_init(thread_t **thread_ptr, thread_pool_t *thread_pool_ptr, int id){
    *thread_ptr = (thread_t *)malloc(sizeof(thread_t));
    if (*thread_ptr == NULL) {
        LOG_ERROR("thread_init(): Could not allocate memory for thread\n");
        return THPOOL_FAIL;
    }
    LOG_DEBUG("thread_init(): Allocated memory for thread %d\n", id);
    (*thread_ptr)->thread_pool_ptr = thread_pool_ptr;
    (*thread_ptr)->id = id;

    pthread_create(&(*thread_ptr)->thread, NULL, (void * (*)(void *))thread_do, (*thread_ptr));
    pthread_detach((*thread_ptr)->thread);
    LOG_DEBUG("thread_init(): Created thread %d in pool \n", id);
    return THPOOL_SUCCESS;
}

/**
 * @brief 线程执行函数
 * 这个函数是线程池中的线程函数，用于执行任务队列中的任务。
 * 线程函数会无限循环地等待新的任务到来，并取出队列中的任务执行。
 * 如果没有新任务到来，则线程将会阻塞等待。
 * 
 * @param thread 需要执行函数的线程
 * @return void* 也许需要返回啥，暂时不知道
 */
static void *thread_do(thread_t *thread){
    // 线程初始化
    // 设置线程名称
    char thread_name[128] = {0};
    snprintf(thread_name, sizeof(thread_name), "thread-%d", thread->id);
#if defined(__linux__)
    prctl(PR_SET_NAME, thread_name);
#elif defined(__APPLE__) && defined(__MACH__)
    pthread_setname_np(thread_name);
#endif
    thread_pool_t *thread_pool_ptr = thread->thread_pool_ptr;
    /* 注册信号 */
    struct sigaction act;
    act.sa_handler = thread_hold;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        LOG_ERROR("thread_do(): Cannot handle SIGUSR1\n");
    }
    // 将线程标记为活跃
    pthread_mutex_lock(&thread_pool_ptr->thcount_lock);
    thread_pool_ptr->num_thread_alive++;
    pthread_mutex_unlock(&thread_pool_ptr->thcount_lock);
    // 线程主循环
    while (threads_keepalive) {
        semaphore_wait(thread->thread_pool_ptr->work_queue.has_jobs);
        if (threads_keepalive) {
            LOG_DEBUG("thread_do(): Thread %d woke up\n", thread->id);
            // 线程池中的线程数加1
            pthread_mutex_lock(&thread->thread_pool_ptr->thcount_lock);
            thread->thread_pool_ptr->num_thread_working++;
            pthread_mutex_unlock(&thread->thread_pool_ptr->thcount_lock);

            // 从工作队列中取出任务并执行
            void (*func_buff)(void *);
            void *arg_buff;
            work_t *work_p;
            work_p = work_queue_pull(&thread->thread_pool_ptr->work_queue);
            if (work_p) {
                func_buff = work_p->function;
                arg_buff = work_p->arg;
                func_buff(arg_buff);
                free(work_p);
            }

            // 线程池中的线程数减1
            pthread_mutex_lock(&thread->thread_pool_ptr->thcount_lock);
            thread->thread_pool_ptr->num_thread_working--;
            LOG_DEBUG("thread_do(): Thread %d is going to sleep\n", thread->id);
            if (!thread->thread_pool_ptr->num_thread_working) {
                pthread_cond_signal(&thread->thread_pool_ptr->threads_all_idle);
            }
            pthread_mutex_unlock(&thread->thread_pool_ptr->thcount_lock);
            LOG_DEBUG("thread_do(): Thread %d is sleeping\n", thread->id);
        }
    }
    LOG_DEBUG("thread_do(): Thread %d is exiting\n", thread->id);
    thread_hold(SIGUSR1);
    return NULL;
}
/**
 * @brief 根据信号指示线程进入休眠状态
 * 
 * 目前只有一个信号SIGUSR1,用于暂停线程
 * 
 * @param sig_id 暂时只有SIGUSR1，所以这个参数没啥用（目前）
 */
static void thread_hold(int sig_id){
    (void)sig_id;
    threads_on_hold = 1;
    while (threads_on_hold) {
        sleep(1);
    }
}
/**
 * @brief 销毁线程
 * 
 * @param thread 需要销毁的线程
 */
static void thread_destroy(thread_t *thread){
    free(thread);
}

/*=====================工作队列相关=====================*/

/**
 * @brief 初始化工作队列
 * 
 * 由于工作队列中采用了双向链表的数据结构，所以需要调用双向链表的初始化函数。
 * @param work_queue 需要初始化的工作队列
 * @return int 成功返回THPOOL_SUCCESS,失败返回THPOOL_FAIL
 */
static int work_queue_init(work_queue_t *work_queue){
    work_queue->list = linked_list_create();
    work_queue->size = 0;
    work_queue->has_jobs = (semaphore_t *)malloc(sizeof(semaphore_t));
    if(work_queue->has_jobs == NULL){
        LOG_ERROR("work_queue_init(): Could not allocate memory for semaphore\n");
        return THPOOL_FAIL;
    }
    pthread_mutex_init(&work_queue->rwmutex, NULL);
    semaphore_init(work_queue->has_jobs, 0);
    return THPOOL_SUCCESS;
}

/**
 * @brief 清空工作队列
 * 
 * @param work_queue 需要清空的工作队列
 */
static void work_queue_clear(work_queue_t *work_queue){
    pthread_mutex_lock(&work_queue->rwmutex);
    linked_list_free(work_queue->list);
    work_queue->size = 0;
    semaphore_reset(work_queue->has_jobs);
    pthread_mutex_unlock(&work_queue->rwmutex);
}

/**
 * @brief 向工作队列中添加任务
 * 
 * @param work_queue 需要添加任务的工作队列
 * @param work 需要添加的任务
 */
static void work_queue_push(work_queue_t *work_queue, work_t *work){
    pthread_mutex_lock(&work_queue->rwmutex);
    linked_list_insert_tail(work_queue->list,(int8*)work,sizeof(work_t));
    work_queue->size++;
    semaphore_post(work_queue->has_jobs);
    pthread_mutex_unlock(&work_queue->rwmutex);
}

/**
 * @brief 从工作队列中取出任务
 * 
 * @param work_queue 需要取出任务的工作队列
 * @return work_t* 取出的任务
 */
static work_t *work_queue_pull(work_queue_t *work_queue){
    work_t *work_p = NULL;
    pthread_mutex_lock(&work_queue->rwmutex);
    LOG_DEBUG("work_queue_pull(): Queue size: %d\n", work_queue->size);
    linked_list_node *node = linked_list_delete_head(work_queue->list);
    work_p = (work_t *)node->data;
    work_queue->size--;
    pthread_mutex_unlock(&work_queue->rwmutex);
    LOG_DEBUG("work_queue_pull(): Pull Success,Queue size: %d\n", work_queue->size);
    return work_p;
}

/**
 * @brief 销毁工作队列
 * 
 * @param work_queue 需要销毁的工作队列
 */
static void work_queue_destroy(work_queue_t *work_queue){
    work_queue_clear(work_queue);
    free(work_queue->has_jobs);
}

/*=====================同步信号相关=====================*/

/**
 * @brief 初始化信号量,目前只有0和1两种状态
 * 
 * @param semaphore 需要初始化的信号量
 * @param init_value 信号量的初始值
 */
static void semaphore_init(semaphore_t *semaphore, int init_value){
    if (init_value < 0 || init_value > 1) {
        LOG_ERROR("semaphore_init(): Could not initialize semaphore\n");
        exit(1);
    }
    pthread_mutex_init(&semaphore->mutex, NULL);
    pthread_cond_init(&semaphore->cond, NULL);
    semaphore->v = init_value;
}

/**
 * @brief 等待信号量
 * 等待信号,直到信号量的值为0
 * 
 * @param semaphore 需要等待的信号量
 */
static void semaphore_wait(semaphore_t *semaphore){
    LOG_DEBUG("semaphore_wait(): Waiting for semaphore\n");
    pthread_mutex_lock(&semaphore->mutex);
    while (semaphore->v != 1) {
        pthread_cond_wait(&semaphore->cond, &semaphore->mutex);
    }
    semaphore->v = 0;
    pthread_mutex_unlock(&semaphore->mutex);
}

/**
 * @brief 发送信号量到至少一个线程
 * 
 * @param semaphore 需要发送的信号量
 */
static void semaphore_post(semaphore_t *semaphore){
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->v = 1;
    pthread_cond_signal(&semaphore->cond);
    pthread_mutex_unlock(&semaphore->mutex);
}

/**
 * @brief 重置信号量
 * 
 * @param semaphore 需要重置的信号量
 */
static void semaphore_reset(semaphore_t *semaphore){
    semaphore_init(semaphore, 0);
}

/**
 * @brief 发送信号量到所有线程
 * 
 * @param semaphore 需要发送的信号量
 */
static void semaphore_post_all(semaphore_t *semaphore){
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->v = 1;
    pthread_cond_broadcast(&semaphore->cond);
    pthread_mutex_unlock(&semaphore->mutex);
}