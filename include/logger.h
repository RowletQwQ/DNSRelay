//日志打印相关,需要实现异步输出日志
#ifndef _LOGGER_H_
#define _LOGGER_H_

#define LOG_LEVEL_DEBUG 0 //调试信息
#define LOG_LEVEL_INFO 1 //一般信息
#define LOG_LEVEL_WARN 2 //警告信息
#define LOG_LEVEL_ERROR 3 //错误信息
#define LOG_LEVEL_FATAL 4 //致命信息

/**
 * @brief 日志打印函数
 * 
 * @param level 日志级别
 * @param format 格式化字符串
 * @param ... 
 */
void write_log(int level, const char *format, ...);
/**
 * @brief 获取当前时间
 * 
 * @return char* 当前时间字符串
 */
char* get_time();

/**
 * @brief 开启日志打印线程
 * 
 * @param log_file 日志文件指针 
 */
void start_log_worker(FILE *log_file);

/**
 * @brief 关闭日志打印线程
 * 
 */
void stop_log_worker();

/**
 * @brief 日志打印线程
 * 
 * @param log_file 日志文件指针
 */
void log_worker(FILE *log_file);



#endif