#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

// 定义变量

// 监听端口和端口地址
SOCKET sock;
struct sockaddr_in any_in_adr, dns_addr;

// DNS服务器地址
char dns_server_4[16] = "114.114.114.114";
char dns_server_6[16];
extern struct list_ops_unit task_pool;

void socket_init(){
    
    // 初始化WSA
    WSADATA wsaData;
    int sta = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (sta != 0) {
        printf("fail to init");
        // write_log(LOG_LEVEL_FATAL,"WSAStartup() failed\n");
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
    dns_addr.sin_addr.s_addr = inet_addr(dns_server_4);
    dns_addr.sin_port = htons(53);
    
    // 绑定通信
    int ret = bind(sock, (struct sockaddr *)&any_in_adr, sizeof(any_in_adr));
    
    if (ret < 0) {
        printf("bind fail");
        // write_log(LOG_LEVEL_FATAL,"bind() failed\n");
        exit(1);
    }
    printf("inited");

}

void socket_close(int sock){
    closesocket(sock);
    // 隐式释放WSA
}

void socket_req_listen(){
    // 报文缓存区
    char recv_message[DNS_MAX_LENGTH] = {0};
    printf("socket_req_listen\n");
    // 循环监听
    while (1) {

        struct sockaddr addr_recv;
        int addr_recv_len = sizeof(addr_recv);
        int ret = recvfrom(sock, recv_message, DNS_MAX_LENGTH, 0,(struct sockaddr *) &addr_recv, &addr_recv_len);
        if (ret == -1) {
            // 写日志
            continue;
        } else if (ret == 0) {
            continue;
        } else {
            // 复制
            struct task task_;
            
            task_.sock_len = addr_recv_len;
            task_.addr = (char *)malloc(addr_recv_len);
            
            memcpy(task_.addr,(char *)&addr_recv,addr_recv_len);
            
            task_.sock_len = addr_recv_len;
            char * message = (char *)malloc(DNS_MAX_LENGTH*sizeof(char));
            memcpy(message, recv_message, ret);
            task_.m_len = ret;
            task_.message = message;
            
            printf("recvfrom() success\n");
            linked_list_insert_tail(task_pool,(int8 *) &task_,sizeof(struct task));

        }

    }
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

    SOCKET send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口
    
    int ret = bind(send_sock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (ret == SOCKET_ERROR) {
        printf("bind failed: %ld\n", WSAGetLastError());
        closesocket(send_sock);
        WSACleanup();
        return 1;
    }

    int send_size =  sendto(sock, message, len, 0, src_addr, addrlen);
    
    
    // 写入文件
    
    FILE *fp = fopen("dns.txt", "wb");
    fwrite(message, 1, len, fp);
    fclose(fp);

    // 依次打印出来
    for (int i = 0; i < len; i++) {
        printf("%02x ", message[i]);
    }
    
    if (send_size == SOCKET_ERROR) {
        // write_log(LOG_LEVEL_ERROR,"send failed\n");
        // 发送失败
        printf("fail to send");
    }
    return send_size;
}

int talk_to_dns(char *message,int len){
    // 绑定新端口
    SOCKET send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // 绑定到本地地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到本地地址
    addr.sin_port = 0; // 端口号设置为 0，系统自动分配随机端口

    // 绑定到DNS服务器
    if(bind(send_sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
        printf("bind failed: %ld\n", WSAGetLastError());
        closesocket(send_sock);
        return -1;
    }
    // 依次打印message
    for (int i = 0; i < len; i++) {
        printf("%02x ", message[i]);
    }
    // 定义dns_addr

    int send_size = sendto(send_sock, message, len, 0, (struct sockaddr *)&dns_addr, sizeof(struct sockaddr));
    
    if (send_size == SOCKET_ERROR) {
        printf("fail to send");
    }else{
        printf("send success\n");
        printf("send_size = %d\n",send_size);
    }
    
    // 设置定时器，超时返回-1
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt(send_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        printf("setsockopt failed\n");
        closesocket(send_sock);
        return -1;
    }

    int addr_len;
    int recv_size = recvfrom(send_sock, message, DNS_MAX_LENGTH, 0, (struct sockaddr *)&dns_addr,&addr_len);   
    
    closesocket(send_sock);
    return recv_size;
}