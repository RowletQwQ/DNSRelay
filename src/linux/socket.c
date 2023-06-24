#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "socket.h"
#include "logger.h"
#include "taskworker.h"
#include "linked_list.h"
// 定义变量
int sock;
struct sockaddr_in any_in_adr, dns_addr;
char dns_server[16];
// 任务池
extern struct linked_list_unit task_pool;

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
    char recv_message[DNS_MAX_LENGTH] = {0};
    // 循环监听
    while (1) {
        struct sockaddr addr_recv;
        int addr_recv_len = sizeof(recv_addr);
        
        int ret = recvfrom(sock, recv_message, DNS_MAX_LENGTH, 0,(struct sockaddr *) &addr_recv, &addr_recv_len);
        
        if (nbytes == -1) {
            // write_log(LOG_LEVEL_ERROR,"recv failed\n");
            exit(EXIT_FAILURE);
        } else if (nbytes == 0) {
            // write_log(LOG_LEVEL_ERROR,"connection is close by peer\n");
            // 接受失败
            continue;
        } else {
            // 复制
            struct task task_;
            task_.addr = addr_recv;
            task_.sock_len = addr_recv_len;
            char * message = (char *)malloc(DNS_MAX_LENGTH*sizeof(char));
            memcpy(message, recv_message, ret);
            task_.m_len = ret;
            
            // 添加到任务池
            linked_list_insert_tail(task_pool, *task_,sizeof(struct task));
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