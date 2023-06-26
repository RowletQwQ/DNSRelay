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
    char *dns_addr = dns.addr;
    if(dns.family != AF_INET && dns.family != AF_INET6){
        LOG_ERROR("get_dns_server() failed");
        exit(1);
    }
    socket_fd = socket(dns.family, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
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

// 监听端口
void socket_req_listen(void * arg){
    (void)arg;
    // 报文缓冲区
    char buf[DNS_MAX_LENGTH];
    LOG_DEBUG("socket_req_listen() start");
    while(1){
        // 接受报文
        socklen_t addr_len;
        struct sockaddr_in6 addr6;
        struct sockaddr_in addr;
        int ret = 0;
        if(get_dns_server().family == AF_INET){
            addr_len = sizeof(addr);
            ret = recvfrom(socket_fd, buf, DNS_MAX_LENGTH, 0, (struct sockaddr *)&addr, &addr_len);
            if(ret <= 0){
                LOG_WARN("recvfrom() failed");
                continue;
            }
        }else{
            addr_len = sizeof(addr6);
            ret = recvfrom(socket_fd, buf, DNS_MAX_LENGTH, 0, (struct sockaddr *)&addr6, &addr_len);
            if(ret <= 0){
                LOG_WARN("recvfrom() failed");
                continue;
            }
        }
        // 将报文放入任务池
        struct task *t = (struct task *)malloc(sizeof(struct task));
        if(t == NULL){
            LOG_ERROR("malloc() failed");
            exit(1);
        }
        t->sock_len = addr_len;
        t->addr = (char*)malloc(addr_len);
        memcpy(t->addr, get_dns_server().family == AF_INET ? (char *)&addr : (char *)&addr6, addr_len);

        t->message = (char*)malloc(DNS_MAX_LENGTH);
        memset(t->message, 0, DNS_MAX_LENGTH);
        memcpy(t->message, buf, DNS_MAX_LENGTH);
        t->m_len = ret;
        LOG_INFO("recvfrom() success");
        linked_list_insert_tail(&task_pool, (char*)t,sizeof(struct task));
    }
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
    any_in_adr.sa_family = AF_INET;
    any_in_adr.sin_addr.s_addr = INADDR_ANY;
    any_in_adr.sin_port = htons(53);

    // 给dns服务器赋值
    memset(&dns_addr, 0, sizeof(dns_addr));
    dns_addr.sa_family = AF_INET;
    memcpy(&dns_addr.sin_addr, dns.addr, sizeof(dns_addr.sin_addr));
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
    memcpy(&dns_addr6.sin6_addr, dns.addr, sizeof(dns_addr6.sin6_addr));
    dns_addr6.sin6_port = htons(53);

    // 绑定接受端口
    if (bind(socket_fd, (struct sockaddr *) &any_in6_adr, sizeof(any_in6_adr)) == -1) {
        LOG_ERROR("bind() failed");
        exit(1);
    }
}

