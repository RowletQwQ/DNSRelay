#include "userfile.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
#include <string.h>
#include "logger.h"
#include "socket.h"
#include "parsedata.h"
#include "db.h"
#include "linked_list.h"

static FILE *fp = NULL;

void init_read_file(const char *file_name) {
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        LOG_ERROR("userfile:open file failed");
        exit(1);
    }
}
void init_write_file(const char *file_name) {
    fp = fopen(file_name, "w+");
    if (fp == NULL) {
        LOG_ERROR("userfile:open file failed");
        exit(1);
    }
}
// 读取数据,数据格式同HOST文件
int read_data(struct domin_table_data **buffer){
    if (fp == NULL) {
        LOG_ERROR("userfile:file not open");
        return -1;
    }
    // 移动文件指针到文件头
    fseek(fp, 0, SEEK_SET);
    // 读取数据,数据为txt格式,每行为一个数据,每行数据格式为:ip domin
    // domain_type 由IP判断
    // 读取数据
    char domain[1024] = {0}, ip[1024] = {0};
    int n = 0;
    struct domin_table_data *buf = (struct domin_table_data *)malloc(sizeof(struct domin_table_data));
    memset(buf, 0, sizeof(struct domin_table_data));
    // 使用双向链表存储数据
    list_ops_unit_t list = linked_list_create();
    while(fscanf(fp, "%s %s", ip, domain) != EOF) {
        //IP地址可能是IPV4或者IPV6,需要判断
        LOG_DEBUG("userfile:ip:%s,domin:%s", ip, domain);
        struct in_addr addr;
        struct in6_addr addr6;
        if (inet_pton(AF_INET, ip, &addr) == 1) {
            buf->record_len = 4;
            buf->record_type = A;
            memcpy(buf->record, &addr, sizeof(struct in_addr));
        } else if (inet_pton(AF_INET6, ip, &addr6) == 1) {
            buf->record_len = 16;
            buf->record_type = AAAA;
            memcpy(buf->record, &addr6, sizeof(struct in6_addr));
        } else {
            //如果既不是IPV4也不是IPV6,则为CNAME
            buf->record_type = CNAME;
            buf->record_len = strlen(ip) + 1;
            strcpy((char*)buf->record, ip);
        }
        strcpy((char*)buf->domin_name, domain);
        //将过期时间设置为-1,表示不过期
        buf->expire_time = -1;
        n++;
        // 将数据插入链表
        linked_list_insert_tail(list, (char*)buf, sizeof(struct domin_table_data));
        memset(buf, 0, sizeof(struct domin_table_data));
    }
    // 将链表转换为数组
    *buffer = (struct domin_table_data *)malloc(sizeof(struct domin_table_data) * n);
    memset(*buffer, 0, sizeof(struct domin_table_data) * n);
    for(int i = 0; i < n; i++){
        struct linked_list_node *node = linked_list_delete_head(list);
        memcpy(&((*buffer)[i]), node->data, node->data_len);
        linked_list_free_node(node);
    }
    return n;
}

// 关闭文件
void close_file() {
    if (fp != NULL) {
        fclose(fp);
    }
}

// 将传入的记录写入文件，格式同HOST文件
void write_data(struct domin_table_data *buffer, int len){
    if(fp == NULL){
        LOG_ERROR("userfile:file not open");
        return;
    }
    // 移动文件指针到文件头
    fseek(fp, 0, SEEK_SET);
    // 将domin_table_data数据写入文件
    for(int i = 0; i < len; i++){
        char ip[1024] = {0};
        if(buffer[i].record_type == A){
            inet_ntop(AF_INET, &buffer[i].record, ip, sizeof(ip));
        }else if(buffer[i].record_type == AAAA){
            inet_ntop(AF_INET6, &buffer[i].record, ip, sizeof(ip));
        }else{
            strcpy(ip, (char*)buffer[i].record);
        }
        fprintf(fp, "%s %s\n", ip, buffer[i].domin_name);
    }
}