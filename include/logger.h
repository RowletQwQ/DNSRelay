//日志打印相关,需要实现异步输出日志
#include <stdio.h>
#ifndef _LOGGER_H_
#define _LOGGER_H_

#define LOG_LEVEL_DEBUG 0 //调试信息
#define LOG_LEVEL_INFO 1 //一般信息
#define LOG_LEVEL_WARN 2 //警告信息
#define LOG_LEVEL_ERROR 3 //错误信息

//宏定义函数
#define LOG_DEBUG(format, ...) write_log(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) write_log(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) write_log(LOG_LEVEL_WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) write_log(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

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
 * @brief 日志打印线程
 * 
 * @param str 日志字符串
 */
void log_worker(const char *str);


#endif