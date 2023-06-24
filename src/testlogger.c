#define LOG_LEVEL_DEBUG 0 //调试信息
#define LOG_LEVEL_INFO 1 //一般信息
#define LOG_LEVEL_WARN 2 //警告信息
#define LOG_LEVEL_ERROR 3 //错误信息

/**
 * @brief 初始化日志打印,需要在main函数中调用,会开启线程池
 * 
 * @param log_file_name 日志文件名
 * @param flag 日志级别
 */
void init_log(const char *log_file_name, int flag);
/**
 * @brief 日志打印函数
 * 日志格式为:日志级别 时间 日志内容
 * 
 * @param level 日志级别
 * @param format 格式化字符串
 * @param ... 
 */
void write_log(int level, const char *format, ...);
/**
 * @brief 获取当前时间
 * 
 * @return 当前时间字符串
 */
const char* get_time();

/**
 * @brief 开启日志打印线程
 * 
 * @param log_file 日志文件指针 
 */

#define LOG_DEBUG(format, ...) write_log(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) write_log(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) write_log(LOG_LEVEL_WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) write_log(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

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
    //log_thread_pool = thpool_create(1);//创建线程池
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
    sprintf(time_str, "%d-%d-%d %d:%d:%d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec);
    return time_str;
}
void log_worker(const char *str){
    assert(log_file != NULL);
    fprintf(log_file, "%s\n", str);
    fflush(log_file);
    free((void *)str);
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
            sprintf(log_str, "[DEBUG] %s %s", time_str, message);
            break;
        case LOG_LEVEL_INFO:
            sprintf(log_str, "[INFO] %s %s", time_str, message);
            break;
        case LOG_LEVEL_WARN:
            sprintf(log_str, "[WARN] %s %s", time_str, message);
            break;
        case LOG_LEVEL_ERROR:
            sprintf(log_str, "[ERROR] %s %s", time_str, message);
            break;
        default:
            break;
    }
    printf("%s\n", log_str);
    //随后将日志字符串传入线程池
   // thpool_add_work(log_thread_pool, (void *)log_worker, (void *)log_str);
    log_worker(log_str);
    free((void *)time_str);
}


int main(){
    init_log("test.log",LOG_LEVEL_DEBUG);
    int a = 1;
    LOG_DEBUG("test debug %d", a);
    LOG_INFO("test info %d", a);
    LOG_WARN("test warn %d", a);
    LOG_ERROR("test error %d", a);
    return 0;
}