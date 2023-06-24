#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "socket.h"
#include "logger.h"
// 定义变量
SOCKET sock;
struct sockaddr_in any_in_adr, dns_addr;
char dns_server[16];

void socket_init(){
    int sta = WSAStartup(MAKEWORD(2, 2), &wsadata);
    
    if (sta != 0) {
        write_log(LOG_LEVEL_FATAL,"WSAStartup() failed\n");
        exit(1);
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    
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
    closesocket(sock);
    // 隐式释放WSA
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