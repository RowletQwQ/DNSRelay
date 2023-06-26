#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "setting.h"
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "logger.h"
#include "thpool.h"
#include "parsedata.h"

// 变量/函数重命名
#if defined(_WIN32) || defined(_WIN64)
typedef SOCKET socket_t;
static inline void close_socket(socket_t sock){
    closesocket(sock);
    WSACleanup();
}

#else
typedef int socket_t;

static inline void close_socket(socket_t sock){
    close(sock);
}

#endif
extern struct list_ops_unit task_pool;
extern thread_pool tasker;
/*=============全局静态变量============*/
static socket_t socket_fd;// socket文件描述符

static struct sockaddr_in any_in_adr;// 本地IPv4地址
static struct sockaddr_in dns_addr;// IPv4 dns服务器地址

static struct sockaddr_in6 any_in6_adr;// 本地IPv6地址
static struct sockaddr_in6 dns_addr6;// IPv6 dns服务器地址
/*=============静态函数原型=============*/
static void socket_init_ipv4(struct my_dns_addr dns);
static void socket_init_ipv6(struct my_dns_addr dns);

/*=============函数实现=================*/
void socket_init(){
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    int sta = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(sta != 0){
        LOG_ERROR("WSAStartup() failed");
        exit(1);
    }
#endif
    // 创建socket
    struct my_dns_addr dns = get_dns_server();
    //char *dns_addr = dns.addr;
    if(dns.family != AF_INET && dns.family != AF_INET6){
        LOG_ERROR("get_dns_server() failed");
        exit(1);
    }
    socket_fd = socket(dns.family, SOCK_DGRAM, 0);
    if (socket_fd == (SOCKET)SOCKET_ERROR) {
        LOG_ERROR("socket() failed");
        exit(1);
    }
    // 分IPV4和IPV6两种情况
    if(dns.family == AF_INET){
        socket_init_ipv4(dns);
    }else{
        socket_init_ipv6(dns);
    }
    LOG_INFO("socket init success");
}

void socket_close(int sock){
    close_socket(sock);
}

int udp_recv(int sockfd, void *buf, int len, struct sockaddr *src_addr, int *addrlen){
    // 默认拿到地址信息
    int nbytes = recvfrom(sockfd, buf, len, 1, src_addr, addrlen);
    if (nbytes == -1) {
        write_log(LOG_LEVEL_ERROR,"recv failed\n");
    }
    return nbytes;
}


int udp_send(int sockfd, const void *buf, int len, const struct sockaddr *dest_addr, int addrlen){
    
    int send_size =  sendto(sockfd, buf, len, 0, dest_addr, addrlen);
    if (send_size == SOCKET_ERROR) {
        write_log(LOG_LEVEL_ERROR,"send failed\n");
    }
    return send_size;
}
/**
 * @brief 初始化IPv4 socket
 * 
 * @param dns dns服务器地址
 */
static void socket_init_ipv4(struct my_dns_addr dns){
    // 给接受端口赋值
    memset(&any_in_adr, 0, sizeof(any_in_adr));
    any_in_adr.sin_family = AF_INET;
    any_in_adr.sin_addr.s_addr = INADDR_ANY;
    any_in_adr.sin_port = htons(53);

    // 给dns服务器赋值
    memset(&dns_addr, 0, sizeof(dns_addr));
    dns_addr.sin_family = AF_INET;
    memcpy(&dns_addr.sin_addr, &dns.u, sizeof(dns_addr.sin_addr));
    dns_addr.sin_port = htons(53);

    // 绑定接受端口
    if (bind(socket_fd, (struct sockaddr *) &any_in_adr, sizeof(any_in_adr)) == -1) {
        LOG_ERROR("bind() failed");
        exit(1);
    }
}

/**
 * @brief 初始化IPv6 socket
 * 
 * @param dns dns服务器地址
 */
static void socket_init_ipv6(struct my_dns_addr dns){
    // 给接受端口赋值
    memset(&any_in6_adr, 0, sizeof(any_in6_adr));
    any_in6_adr.sin6_family = AF_INET6;
    any_in6_adr.sin6_addr = in6addr_any;
    any_in6_adr.sin6_port = htons(53);

    // 给dns服务器赋值
    memset(&dns_addr6, 0, sizeof(dns_addr6));
    dns_addr6.sin6_family = AF_INET6;
    memcpy(&dns_addr6.sin6_addr, &dns.u, sizeof(dns_addr6.sin6_addr));
    dns_addr6.sin6_port = htons(53);

    // 绑定接受端口
    if (bind(socket_fd, (struct sockaddr *) &any_in6_adr, sizeof(any_in6_adr)) == -1) {
        LOG_ERROR("bind() failed");
        exit(1);
    }
}

void socket_req_listen(){
    // 报文缓存区
    // char recv_message[DNS_MAX_LENGTH] = {0};
    LOG_INFO("socket_req_listen");
    // 提前创建任务
    struct task*task_ = (struct task *)malloc(sizeof(struct task));
    task_->message = (char *)malloc(DNS_MAX_BUF *sizeof(char));
    // 循环监听
    while (1) {
        struct sockaddr addr_recv;
        int addr_recv_len = sizeof(addr_recv);
        int ret = recvfrom(socket_fd, task_->message, DNS_MAX_LENGTH, 0,(struct sockaddr *) &addr_recv, &addr_recv_len);
        
        if (ret == -1) {
            // 写日志
            continue;
        } else if (ret == 0) {
            continue;
        } else {
            task_->m_len = ret;
            // 复制
            task_->addr = addr_recv;
            task_->sock_len = addr_recv_len;
            thpool_add_work(tasker, (void *)taskworker, (void *)task_);
            task_ = (struct task *)malloc(sizeof(struct task));
            task_->message = (char *)malloc(DNS_MAX_BUF *sizeof(char));
        }
    }
    socket_close(socket_fd);
}
int send_to_client(char *message,int len,struct sockaddr *src_addr, int addrlen){
    //打印src_addr对应的地址
    struct sockaddr_in *src_addr_in = (struct sockaddr_in *)src_addr;
    char *src_ip = inet_ntoa(src_addr_in->sin_addr);
    LOG_INFO("send to client ip:%s",src_ip);
    socket_t send_sock= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    struct sockaddr_in addr;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口
    
    int ret = bind(send_sock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (ret == SOCKET_ERROR) {
        LOG_ERROR("bind() failed");
        socket_close(send_sock);
        return 1;
    }

    int send_size =  sendto(socket_fd, message, len, 0, src_addr, addrlen);
    

    if (send_size == SOCKET_ERROR) {
        LOG_ERROR("sendto() failed");
    }

    return send_size;
}

int talk_to_dns(char *message,int len,struct sockaddr src_addr, int addrlen){
    socket_t send_sock_temp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // 绑定新端口

    // 绑定到本地地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口

    // 绑定到DNS服务器
    if(bind(send_sock_temp, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
        LOG_ERROR("DNS BIND FAIL");
        socket_close(send_sock_temp);
        return -1;
    }

    // 定义dns_addr
    int send_size = sendto(send_sock_temp, message, len, 0, (struct sockaddr *)&dns_addr, sizeof(struct sockaddr));

    if (send_size == SOCKET_ERROR) {
        LOG_ERROR("sendto() failed");
    }else{
        LOG_DEBUG("sendto() success, send_size: %d", send_size);
    }
    
    // 设置定时器，超时返回 -1
    struct timeval timeout;
    timeout.tv_sec = 3;
    if (setsockopt(send_sock_temp, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        printf("setsockopt failed\n");
        socket_close(send_sock_temp);
        return -1;
    }
    
    printf("setsockopt success\n");

    int recv_size = 0;
    recv_size = recvfrom(send_sock_temp, message, DNS_MAX_LENGTH, 0,NULL,NULL);
    
    // 发送给客户端
    if(recv_size != SOCKET_ERROR){
        printf("recv success %d\n",recv_size);
        send_to_client(message,recv_size,&src_addr,addrlen);
    }

    socket_close(send_sock_temp);
    
    return recv_size;
}