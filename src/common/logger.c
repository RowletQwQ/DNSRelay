#include "logger.h"
#include "thpool.h"

static int thread_pool_size = 0;
static int thread_pool_alive = 0;
static thread_pool log_thread_pool = NULL;
static FILE *log_file = NULL;
static int log_flag = LOG_LEVEL_ERROR;
//初始化日志打印,需要在main函数中调用,会开启线程池
void init_log(const char *log_file_name, int flag){
    log_file = fopen(log_file_name, "a+");
    if(log_file == NULL){
        LOG_ERROR("open log file failed");
        exit(1);
    }
    log_flag = flag;
    log_thread_pool = thpool_create(1);//创建线程池
}
//日志打印函数
void write_log(int level, const char *format, ...){
    va_list args;
    va_start(args, format);
    char *log = (char *)malloc(sizeof(char) * 1024);
    if(log == NULL){
        printf("malloc failed\n");
        exit(1);
    }
    vsprintf(log, format, args);
    va_end(args);
}