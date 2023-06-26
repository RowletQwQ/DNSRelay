#include "logger.h"
#include "linked_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stddef.h>

static list_ops_unit_t msg_queue;
#define DISABLE_MUTI_THREAD
/*=============线程相关=================*/
#include <pthread.h>
typedef pthread_t thread_t;
typedef pthread_rwlock_t rwlock_t;
static inline void rwlock_init(rwlock_t *lock){
    pthread_rwlock_init(lock, NULL);
}
static inline void rwlock_rdlock(rwlock_t *lock){
    pthread_rwlock_rdlock(lock);
}
static inline void rwlock_wrlock(rwlock_t *lock){
    pthread_rwlock_wrlock(lock);
}
static inline void rwlock_unlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}
static inline void rwlock_wr_unlock(rwlock_t *lock){
    pthread_rwlock_unlock(lock);
}
static inline void rwlock_destroy(rwlock_t *lock){
    pthread_rwlock_destroy(lock);
}
static inline void thread_create(thread_t *thread, void *(*func)(void *), void *arg){
    pthread_create(thread, NULL, func, arg);
}
static inline void thread_join(thread_t thread){
    pthread_join(thread, NULL);
}

/*===============全局变量==================*/
//static thread_t log_thread = NULL;
static int std_status = 0;   
static FILE *log_file = NULL;
static int log_flag = LOG_LEVEL_ERROR;
static rwlock_t log_lock;
static volatile int log_thread_status = 0;

/*===============静态函数原型===============*/
static void* log_worker(void *arg);
static const char* get_time();
static void log_worker_nolock(const char *str);


//初始化日志打印,需要在main函数中调用,会开启线程池
void init_log(const char *log_file_name, int flag,int std_flag){
    LOG_DEBUG("logger:init log");
    log_file = fopen(log_file_name, "w+");
    if(log_file == NULL){
        LOG_ERROR("logger:open log file failed");
        exit(1);
    }
    log_flag = flag;
    std_status = std_flag;
    //printf("logger:log init success\n");
#ifdef DISABLE_MUTI_THREAD
    
#else
    pthread_t log_thread;
    msg_queue = linked_list_create();
    log_thread_status = 1;
    
    pthread_create(&log_thread,NULL, log_worker, NULL);
    rwlock_init(&log_lock);
    //printf("logger:log thread init success\n");
    pthread_detach(log_thread);
    pthread_join(log_thread, NULL);
#endif
}
//日志打印函数,最大长度1024
void write_log(int level, const char *format, ...){
    if(level < log_flag){
        return;
    }
    char *log_str = (char *)malloc(sizeof(char) * 2048);
    if(log_str == NULL){
        printf("logger:malloc failed\n");
        exit(1);
    }
    memset(log_str, 0, sizeof(char) * 2048);
    const char *time_str = get_time();

    char message[1024];
    memset(message, 0, sizeof(char) * 1024);
    va_list args;
    va_start(args, format);
    vsprintf(message, format, args);
    va_end(args);
    
    switch (level)
    {
        case LOG_LEVEL_DEBUG:
            sprintf(log_str, "%s [DEBUG] %s", time_str, message);
            break;
        case LOG_LEVEL_INFO:
            sprintf(log_str, "%s [INFO] %s", time_str, message);
            break;
        case LOG_LEVEL_WARN:
            sprintf(log_str, "%s [WARN] %s", time_str, message);
            break;
        case LOG_LEVEL_ERROR:
            sprintf(log_str, "%s [ERROR] %s", time_str, message);
            break;
        default:
            break;
    }
    
    //随后将日志字符串传入线程池
#ifndef DISABLE_MUTI_THREAD
    if(log_thread_status == 0){
        log_worker_nolock(log_str);
    }else{
        rwlock_wrlock(&log_lock);
        linked_list_insert_tail(msg_queue, log_str,2048);
        rwlock_wr_unlock(&log_lock);
    }
#else
    log_worker_nolock(log_str);
#endif
}

/**
 * @brief 获取当前时间
 * 
 * @return const char* 
 */
const char* get_time(){
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char *time_str = (char *)malloc(sizeof(char) * 100);
    if(time_str == NULL){
        printf("logger:malloc failed\n");
        exit(1);
    }
    memset(time_str, 0, sizeof(char) * 100);
    sprintf(time_str, "%d-%02d-%02d %02d:%02d:%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec);
    return time_str;
}

/**
 * @brief 日志线程函数
 * 
 */
void* log_worker(void *arg){
    //加读写锁
    (void)arg;
    while(log_thread_status){
        rwlock_rdlock(&log_lock);
        struct linked_list_node * node = linked_list_delete_head(msg_queue);
        if(node != NULL){
            log_worker_nolock(node->data);
        }
        rwlock_unlock(&log_lock);
        if(node == NULL){
            //Sleep(100);
            usleep(100);
            continue;
        }
        linked_list_free_node(node);
    }
    return NULL;
}

/**
 * @brief 无锁日志打印函数
 * 
 * @param str 
 */
void log_worker_nolock(const char *str){
    assert(log_file != NULL);
    fprintf(log_file, "%s\n", (char*)str);
    if(std_status == 1){
        printf("%s\n", str);
    }
    fflush(log_file);
}

/**
 * @brief 关闭日志打印
 * 
 */
void close_log(){
    while(linked_list_get_head(msg_queue) != NULL){}
    //sleep(5);
    log_thread_status = 0;
    rwlock_destroy(&log_lock);
    linked_list_free(msg_queue);
    fclose(log_file);
    //printf("logger:log close success\n");
}
