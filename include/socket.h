//此处声明UDP和TCP的socket相关操作函数
#include "datatype.h"

#ifndef _SOCKET_H_
#define _SOCKET_H_
//常量定义
// #define SOCKET_ERROR -1
// #define INVALID_SOCKET -1
//地址族定义
//#define AF_INET 2 //IPv4地址族
//#define AF_INET6 23 //IPv6地址族
//#define AF_UNSPEC 0 //未指定地址族
//套接字类型定义
//#define SOCK_STREAM 1 //流式套接字
//#define SOCK_DGRAM 2 //数据报套接字
//#define SOCK_RAW 3 //原始套接字
//#define SOCK_RDM 4 //可靠数据报套接字
// DNS报文最大长度
#define DNS_MAX_LENGTH 512

struct sockaddr;

/**
 * @brief 初始化socket，包含sock标识符，地址信息，端口号等
 */

void socket_init();

/**
 * @brief 关闭socket
 * 
 * @param sock 套接字描述符
 */

void socket_close();


/**
 * @brief 监听请求端口 将dns请求加入到任务池中
 * @param arg 占位符
 */
void socket_req_listen();

//UDP
/**
 * @brief 创建UDP套接字
 * 
 * @param sockfd 套接字描述符
 * @param buf 待发送数据
 * @param len 待发送数据长度
 * @param dest_addr 目标地址
 * @param addrlen 目标地址长度
 * @return int 发送成功返回发送的字节数，失败返回SOCKET_ERROR
 */
int udp_send(int sockfd, const void *buf, int len, 
             const struct sockaddr *dest_addr, int addrlen);

/**
 * @brief 接收UDP套接字，接收数据存放在buf中，源地址存放在src_addr中
 * 
 * @param sockfd 套接字描述符
 * @param buf 接收缓冲区
 * @param len 接收缓冲区长度
 * @param src_addr 源地址
 * @param addrlen 源地址长度
 * @return int 接收成功返回接收的字节数，失败返回SOCKET_ERROR
 */
int udp_recv(int sockfd, void *buf, int len, 
             struct sockaddr *src_addr, int *addrlen);

/**
 * @brief 将报文发送给客户端
 * 
 * @param message 报文
 * @param len 报文长度
 * @param src_addr 客户端地址
 * @param addrlen 客户端地址长度
 * @return int 
 */
int send_to_client(char *message,int len,struct sockaddr *src_addr, int addrlen);


//TCP
//这个程序是Server端,故需要提供四个函数
/**
 * @brief TCP套接字绑定
 * 
 * @param domain 指定套接字的协议族(IPv4/IPv6)
 * @param type 指定套接字的类型(流式套接字/数据报套接字)
 * @param protocol 指定套接字所使用的协议
 * @return int 
 */
int tcp_server_create(int domain, int type, int protocol);

/**
 * @brief TCP套接字绑定
 * 
 * @param sockfd 套接字描述符
 * @param addr 地址信息
 * @param addrlen 地址信息长度
 * @return int 成功返回0，失败返回SOCKET_ERROR 
 */
int tcp_server_bind(int sockfd, const struct sockaddr *addr, 
                    int addrlen);


/**
 * @brief 监听TCP套接字
 * 
 * @param sockfd 套接字描述符
 * @param backlog 最大连接数
 * @return int 成功返回0，失败返回SOCKET_ERROR
 */
int tcp_server_listen(int sockfd, int backlog);

/**
 * @brief 接收TCP套接字，返回新的套接字描述符
 * 
 * @param sockfd 套接字描述符
 * @param addr 地址信息
 * @param addrlen 地址信息长度
 * @return int 成功返回新的套接字描述符，失败返回SOCKET_ERROR
 */
int tcp_server_accept(int sockfd, struct sockaddr *addr, int *addrlen);

/**
 * @brief 扔给DNS服务器获得报文 超时返回 可以有多个DNS服务器
 * 
 * @param message 指向请求报文的指针
 * @param len 报文长度
 * 
 * @return int 返回新的报文长度，并且报文已经被修改 超时返回-1
 */
int talk_to_dns(char *message,int len,struct sockaddr src_addr, int addrlen);


#endif
