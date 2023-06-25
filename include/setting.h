// 解析命令行设置
#ifndef _SETTING_H_
#define _SETTING_H_

// 声明变量
typedef struct setting setting_t;
struct sockaddr_in;
struct my_dns_addr{
    unsigned short family;
    union 
    {
        unsigned char Byte[16];
        unsigned short Word[8];
    }u;
};
/**
 * @brief 输出hello信息
 * 
 */
void say_hello();

/**
 * @brief 解析命令行设置
 * 
 * @param argc 参数个数
 * @param argv 参数列表
 */
void parse_args(int argc, char *argv[]);

/**
 * @brief 输出帮助信息
 * 
 */
void say_help();

/**
 * @brief 获得调试等级
 * 
 * @return int 
 */
int get_debug_level();

/**
 * @brief 获得DNS服务器地址
 * 
 * @return struct dns_addr DNS服务器地址
 */
struct my_dns_addr get_dns_server();

/**
 * @brief 获得DNS服务器地址(字符串)
 * 
 * @return char* DNS服务器地址
 */
char *get_dns_server_ip();
/**
 * @brief 获得用户自定义的文件
 * 
 * @return char* 用户自定义的HOST文件
 */
char *get_user_file();

/**
 * @brief 获得日志文件名称
 * 
 * @return char* 日志文件名称
 */
char *get_log_file();

/**
 * @brief 获取DNS类型
 * @return int AF_INET 或者 AF_INET6
 */
int get_dns_type();
#endif