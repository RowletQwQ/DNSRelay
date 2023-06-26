#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thpool.h"
#include "parsedata.h"
#include "logger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
SOCKET sock;
#endif
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
int sock;
#define SOCKET_ERROR SO_ERROR
#define SOCK_DGRAM 2
#endif

// 定义变量
extern thread_pool tasker;
// 监听端口和端口地址

struct sockaddr_in any_in_adr, dns_addr;

// DNS服务器地址
char dns_server_4[16] = "114.114.114.114";
char dns_server_6[16];

void socket_init(){

    #ifdef _WIN32
        // 初始化WSA
        WSADATA wsaData;
        int sta = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (sta != 0) {
            LOG_ERROR("SOCKET INIT FAIL");
            exit(1);
        }
        sock = socket(AF_INET, SOCK_DGRAM,0);
    #else
        sock = socket(AF_INET, SOCK_DGRAM, 0);
    #endif    
    
    // 给接受端口赋值
    memset(&any_in_adr, 0, sizeof(any_in_adr));
    any_in_adr.sin_family = AF_INET;
    any_in_adr.sin_addr.s_addr = INADDR_ANY;
    any_in_adr.sin_port = htons(53);
    
    // 给dns服务器赋值
    memset(&dns_addr, 0, sizeof(dns_addr));
    dns_addr.sin_family = AF_INET;
    dns_addr.sin_addr.s_addr = inet_addr(dns_server_4);
    dns_addr.sin_port = htons(53);
    
    // 绑定通信
    int ret = bind(sock, (struct sockaddr *)&any_in_adr, sizeof(any_in_adr));
    
    if (ret < 0) {

        LOG_ERROR("SOCKET BIND FAIL");
        
        exit(1);
    }
}

void socket_close(){
    #ifdef _WIN32
        // 初始化WSA
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif    
    // 隐式释放WSA
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
        int ret = recvfrom(sock, task_->message, DNS_MAX_LENGTH, 0,(struct sockaddr *) &addr_recv, &addr_recv_len);
        
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
    socket_close();
}

int udp_recv(int sockfd, void *buf, int len, struct sockaddr *src_addr, int *addrlen){
    // 默认拿到地址信息
    int nbytes = recvfrom(sockfd, buf, len, 1, src_addr, addrlen);
    if (nbytes == -1) {
        
    }
    return nbytes;
}

// 绑定给不同的端口
int udp_send(int sockfd, const void *buf, int len, const struct sockaddr *dest_addr, int addrlen){
    
    int send_size =  sendto(sockfd, buf, len, 0, dest_addr, addrlen);
    if (send_size == SOCKET_ERROR) {
        // write_log(LOG_LEVEL_ERROR,"send failed\n");

    }
    return send_size;
}

int send_to_client(char *message,int len,struct sockaddr *src_addr, int addrlen){
    //打印src_addr对应的地址
    struct sockaddr_in *src_addr_in = (struct sockaddr_in *)src_addr;
    char *src_ip = inet_ntoa(src_addr_in->sin_addr);
    #ifdef _WIN32
        SOCKET send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    #else
        int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    #endif    
    
    struct sockaddr_in addr;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口
    
    int ret = bind(send_sock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (ret == SOCKET_ERROR) {
        #ifdef _WIN32
            LOG_ERROR("SOCKET BIND FAIL");
            closesocket(send_sock);
        #else
            close(send_sock);
        #endif    
        return 1;
    }

    int send_size =  sendto(sock, message, len, 0, src_addr, addrlen);

    if (send_size == SOCKET_ERROR) {
        LOG_WARN("send failed\n");
    }

    return send_size;
}

int talk_to_dns(char *message,int len,struct sockaddr src_addr, int addrlen){
    
    #ifdef _WIN32
        SOCKET send_sock_temp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);        
    #else
        int send_sock_temp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    #endif    
    // 绑定新端口

    // 绑定到本地地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口

    // 绑定到DNS服务器
    if(bind(send_sock_temp, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
        #ifdef _WIN32
            LOG_ERROR("DNS BIND FAIL");
            closesocket(send_sock_temp);
        #else
            close(send_sock_temp);
        #endif    
        return -1;
    }

    // 定义dns_addr
    int send_size = sendto(send_sock_temp, message, len, 0, (struct sockaddr *)&dns_addr, sizeof(struct sockaddr));

    if (send_size == SOCKET_ERROR) {
        LOG_WARN("fail to send a pocket from %s to dns!",inet_ntoa(((struct sockaddr_in *)&src_addr)->sin_addr));
    }
    
    // 设置定时器，超时返回 -1
    struct timeval timeout;
    timeout.tv_sec = 1;
    if (setsockopt(send_sock_temp, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        LOG_WARN("fail to set a sockopt!");
        #ifdef _WIN32
            closesocket(send_sock_temp);
        #else
            close(send_sock_temp);
        #endif    
        return -1;
    }

    int recv_size = 0;
    recv_size = recvfrom(send_sock_temp, message, DNS_MAX_LENGTH, 0,NULL,NULL);
    
    // 发送给客户端
    if(recv_size != SOCKET_ERROR){
        LOG_INFO("send a pocket to client %s",inet_ntoa(((struct sockaddr_in *)&src_addr)->sin_addr));
        send_to_client(message,recv_size,&src_addr,addrlen);
    }

    #ifdef _WIN32
            closesocket(send_sock_temp);
    #else
            close(send_sock_temp);
    #endif    
    
    return recv_size;
}