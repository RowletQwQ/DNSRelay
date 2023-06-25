#include "userfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "logger.h"
#include "socket.h"
#include "parsedata.h"
#include "db.h"

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
int read_data(struct domin_table_data *buffer, int len){
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
    while(fscanf(fp, "%s %s", ip, domain) != EOF) {
        //IP地址可能是IPV4或者IPV6,需要判断
        LOG_DEBUG("userfile:ip:%s,domin:%s", ip, domain);
        struct in_addr addr;
        struct in6_addr addr6;
        if (inet_pton(AF_INET, ip, &addr) == 1) {
            buffer[n].record_type = A;
            memcpy(&buffer[n].record, &addr, sizeof(struct in_addr));
        } else if (inet_pton(AF_INET6, ip, &addr6) == 1) {
            buffer[n].record_type = AAAA;
            memcpy(&buffer[n].record, &addr6, sizeof(struct in6_addr));
        } else {
            //如果既不是IPV4也不是IPV6,则为CNAME
            buffer[n].record_type = CNAME;
            strcpy((char*)buffer[n].record, ip);
        }
        strcpy((char*)buffer[n].domin_name, domain);
        //将过期时间设置为-1,表示不过期
        buffer[n].expire_time = -1;
        n++;
        if (n >= len) {
            break;
        }
        
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