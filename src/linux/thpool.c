#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>


#include "thpool.h"
#include "logger.h"
#include "linked_list.h"
#include "logger.h"

//Linux下的线程池实现
static volatile int threads_keepalive;// 线程池中线程的最长空闲时间
static volatile int threads_on_hold;// 线程池中线程的数量
/*============================== 结构体 =============================*/
/* 信号量 */
typedef struct semaphore {
    pthread_mutex_t mutex;// 互斥锁,实现二元信号量的互斥访问
    pthread_cond_t cond;// 条件变量,实现二元信号量的等待和唤醒操作
    int count;// 整数变量,表示当前信号量的值
} semaphore_t;

/* 工作 */
typedef struct work {
    struct work *prev; // 指向上一个任务
    void (*function)(void *arg); // 函数指针,指向任务函数
    void *arg; // 任务函数的参数
} work_t;
/* 工作队列,基于双向链表实现 */
typedef struct work_queue_node {
    pthread_mutex_t mutex; // 互斥锁,在队列中插入或删除节点时需要加锁
    semaphore_t *semaphore; // 一个二元的信号量标识符
    int length; // 队列中的节点数量
} work_queue_node_t;
typedef struct list_ops_unit work_queue_t;

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
    phread_mutex_t thcount_lock; // 互斥锁,在线程技术等时候使用
    phread_cond_t threads_all_idle; // 条件变量,thpool_wait()函数中使用
    work_queue_t *work_queue; // 工作队列

} thread_pool_t;

/*====================内部函数原型====================*/

static int thread_init(thread_t **thread, thread_pool thread_pool_ptr, int id);
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
    if(jobqueue_init(&thread_pool_ptr->work_queue) == THPOOL_FAIL){
        LOG_ERROR("thpool_create(): Could not allocate memory for work queue\n");
        free(thread_pool_ptr);
        return NULL;
    }

    // 初始化池中的线程
    thread_pool_ptr->threads = (thread_t **)malloc(sizeof(thread_t *) * thread_num);
    if (thread_pool_ptr->threads == NULL) {
        LOG_ERROR("thpool_create(): Could not allocate memory for threads\n");
        jobqueue_destroy(&thread_pool_ptr->work_queue);
        free(thread_pool_ptr);
        return NULL;
    }

    pthread_mutex_init(&(thread_pool_ptr->thcount_lock), NULL);
    pthread_cond_init(&(thread_pool_ptr->threads_all_idle), NULL);

    // 线程初始化
    for (int n = 0; n < thread_num; n++) {
        thread_init(&(thread_pool_ptr->threads[n]), thread_pool_ptr, n);
        LOG_DEBUG("thpool_create(): Created thread %d in pool \n", n);
        if (pthread_detach(thread_pool_ptr->threads[n]->thread) != 0) {
            LOG_ERROR("thpool_create(): Could not detach thread %d\n", n);
            jobqueue_destroy(&thread_pool_ptr->work_queue);
            thpool_destroy(thread_pool_ptr);
            return NULL;
        }
    }

    // 等待所有线程启动
    while (thread_pool_ptr->num_thread_alive != thread_num) {}
    return thread_pool_ptr;
}