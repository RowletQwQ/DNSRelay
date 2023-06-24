#include "logger.h"

#ifndef DISABLE_MUTI_THREAD
#include "thpool.h"
static int thread_pool_size = 0;
static int thread_pool_alive = 0;
static thread_pool log_thread_pool = NULL;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>


static FILE *log_file = NULL;
static int log_flag = LOG_LEVEL_ERROR;
//初始化日志打印,需要在main函数中调用,会开启线程池
void init_log(const char *log_file_name, int flag){
    log_file = fopen(log_file_name, "a+");
    if(log_file == NULL){
        LOG_ERROR("logger:open log file failed");
        exit(1);
    }
    log_flag = flag;
#ifndef DISABLE_MUTI_THREAD
    log_thread_pool = thpool_create(1);//创建线程池
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
    printf("%s\n", log_str);
    //随后将日志字符串传入线程池
#ifndef DISABLE_MUTI_THREAD
    if(log_thread_pool != NULL){
        thpool_add_work(log_thread_pool, (void *)log_worker, (void *)log_str);
    }else{
        log_worker(log_str);
    }
#else
    log_worker(log_str);
#endif
}

const char* get_time(){
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char *time_str = (char *)malloc(sizeof(char) * 20);
    if(time_str == NULL){
        printf("logger:malloc failed\n");
        exit(1);
    }
    sprintf(time_str, "%d-%02d-%02d %02d:%02d:%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec);
    return time_str;
}

void log_worker(const char *str){
    assert(log_file != NULL);
    fprintf(log_file, "%s\n", str);
    fflush(log_file);
    free((void *)str);
}