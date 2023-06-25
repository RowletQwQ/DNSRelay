#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "socket.h"
#include "logger.h"
//IPv4地址结构
struct in_addr{
    uint32 s_addr; //IP地址,网络字节序
};

//IPv6地址结构
struct in6_addr{
    uint8 s6_addr[16]; //IP地址,网络字节序
};
//sockaddr结构体，用于存储地址信息,可以用memcpy直接拷贝
struct sockaddr{
    uint16 sa_family; //地址族
    char sa_data[14]; //14字节协议地址，包含该套接字的IP地址和端口号
};
//IPv4 sock地址结构
struct sockaddr_in{
    uint16 sin_family; //地址族类型,必须为AF_INET,表示IPv4地址
    uint16 sin_port; //端口号,必须为网络字节序
    struct in_addr sin_addr; //IP地址
    byte sin_zero[8]; //填充0以保持与sockaddr结构的长度相同
};
//IPv6 sock地址结构
struct sockaddr_in6{
    uint16 sin6_family; //地址族类型,必须为AF_INET6,表示IPv6地址
    uint16 sin6_port; //端口号,必须为网络字节序
    uint32 sin6_flowinfo; //流标识
    struct in6_addr sin6_addr; //IP地址
    uint32 sin6_scope_id; //范围ID
};

// 定义变量
int sock;
struct sockaddr_in any_in_adr, dns_addr;
char dns_server[16];

void socket_init(){
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock == -1) {
        write_log(LOG_LEVEL_FATAL,"socket() failed\n");
        exit(1);
    }
    
    // 给接受端口赋值
    memset(&any_in_adr, 0, sizeof(any_in_adr));
    any_in_adr.sin_family = AF_INET;
    any_in_adr.sin_addr.s_addr = INADDR_ANY;
    any_in_adr.sin_port = htons(53);
    
    // 给dns服务器赋值
    memset(&dns_addr, 0, sizeof(dns_addr));
    dns_addr.sin_family = AF_INET;
    dns_addr.sin_addr.s_addr = inet_addr(dns_server);
    dns_addr.sin_port = htons(53);
    
    // 绑定通信
    int ret = bind(sock, (struct sockaddr *)&any_in_adr, sizeof(any_in_adr));
    if (ret < 0) {
        write_log(LOG_LEVEL_FATAL,"bind() failed\n");
        exit(1);
    }
}

void socket_close(int sock){
    close(sock);
}

void socket_req_listen(){
    int new_socket;// 接收到的req
    // 建立监听关系 3 是最大监听数
    if (listen(sock, 3) < 0) {
        write_log(LOG_LEVEL_FATAL,"listen build failed\n");
        exit(1);
    }
    
    // 循环监听
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            write_log(LOG_LEVEL_FATAL,"accept failed\n");
            continue;
        }

        // 从新的Socket描述符中读取数据
        int nbytes = recv(new_socket, buffer, 1024, 0);
        
        if (nbytes == -1) {
            write_log(LOG_LEVEL_ERROR,"recv failed\n");
            exit(EXIT_FAILURE);
        } else if (nbytes == 0) {
            write_log(LOG_LEVEL_ERROR,"connection is close by peer\n");
        } else {
            // 把监听到的端口加入到任务池
            
            write_log(LOG_LEVEL_INFO,"new request locate in %d sock", new_socket);
        }
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