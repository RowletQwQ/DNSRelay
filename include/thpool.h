//线程池相关函数声明
#ifndef THPOOL_H
#define THPOOL_H

#define THPOOL_SUCESS 0
#define THPOOL_FAIL -1

typedef struct thread_pool_t* thread_pool;

/**
 * @brief 创建线程池
 * 
 * 初始化一个线程池,在创建完指定数量的线程后，这个函数才会返回
 * 
 * @param thread_num 线程池中线程的数量
 * @return thread_pool 线程池指针
 */
thread_pool thpool_create(int thread_num);

/**
 * @brief 把任务添加到线程池中
 * 
 * 把一个任务加入到线程池队列中
 * 
 * @example
 * void print_num(int num){
 *     printf("%d\n", num);
 * }
 *
 *  int main(){
 *     ..
 *     int a = 10;
 *     thpool_add_work(thpool, (void*)print_num, (void*)a);
 *     ..
 * }
 * 
 * @param pool 待添加任务的线程池
 * @param func 任务函数
 * @param arg 任务函数的参数
 * @return int 0表示成功,其他表示失败
 */
int thpool_add_work(thread_pool pool, void *(*func)(void *), void *arg);

/**
 * @brief 等待线程池中所有任务结束
 * 
 * 等待线程池中所有任务结束
 * 
 * @param pool 线程池指针
 */
void thpool_wait(thread_pool pool);

/**
 * @brief 暂停线程池中所有任务
 * 
 * @param pool 线程池指针
 */
void thpool_pause(thread_pool pool);

/**
 * @brief 恢复线程池中所有任务
 * 
 * @param pool 线程池指针
 */
void thpool_resume(thread_pool pool);

/**
 * @brief 销毁线程池
 * 
 * @param pool 线程池指针
 */
void thpool_destroy(thread_pool pool);

/**
 * @brief 获取线程池中线程的数量
 * 
 * @param pool 线程池指针
 * @return int 线程池中线程的数量
 */
int thpool_get_thread_num(thread_pool pool);

#endif