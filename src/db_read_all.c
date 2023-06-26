#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "db.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
#define file_name "db_data.txt"

// 将数据库中的所有数据读取到一个文件中
int main() {
    // 先调用db中的query_all_dns_record函数
    struct domin_table_data *data = NULL;
    int32 cnt = query_all_dns_record(&data);

    // 将数据写入文件 (覆盖写入)
    FILE *fp = fopen(file_name, "w");
    assert(fp != NULL);

    for (int32 i = 0; i < cnt; i++) {
        fprintf(fp, "%s %d %d %lld", data[i].domin_name, data[i].record_type, data[i].record_len, data[i].expire_time);
        char ip[1024] = {0};
        if(data[i].record_type == A){
            inet_ntop(AF_INET, &data[i].record, ip, sizeof(ip));
        }else if(data[i].record_type == AAAA){
            inet_ntop(AF_INET6, &data[i].record, ip, sizeof(ip));
        }else{
            strcpy(ip, (char*)data[i].record);
        }
        fprintf(fp, " %s\n", ip);
    }

    fclose(fp);
    
    return 0;
}