#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif
#include "dao.h"
#include "trie.h"
#include "db.h"
#include "logger.h"
#include "userfile.h"
#include "thpool.h"
//测试dao层工作是否正常
//测试通过读取userfile.txt文件实现,先读取userfile.txt
//调用dao层接口函数进行写入，随后进行查询
void add_task(void* args){
    struct domin_table_data *buffer = (struct domin_table_data *)args;
    int ret = add_record(buffer->domin_name, buffer->record_type, buffer->record,256,30);
    if(ret == -1){
            LOG_ERROR("dao_insert error");
        }else{
            char ip[INET6_ADDRSTRLEN] = {0};
            if(buffer->record_type == A){
                inet_ntop(AF_INET, buffer->record, ip, INET_ADDRSTRLEN);
            }else if(buffer->record_type == AAAA){
                inet_ntop(AF_INET6, buffer->record, ip, INET6_ADDRSTRLEN);
            }else{
            //CNAME or NS
                strcpy(ip, (char*)buffer->record);
            }
            LOG_INFO("dao_insert success,domain:%s,record_type:%d,record:%s",buffer->domin_name,buffer->record_type,ip);
        }
}
int main(){
    init_log("log.txt", LOG_LEVEL_DEBUG, 1);
    init_read_file("userfile.txt");
    init_dao();
    struct domin_table_data *buffer = NULL;
    int len = read_data(&buffer);
    printf("len:%d\n", len);
    //调用dao层接口写入
    // 开线程池
    thread_pool pool = thpool_create(12);
    for(int i = 0; i < len; i++){
        thpool_add_work(pool, add_task, (void*)&buffer[i]);
    }
    thpool_wait(pool);
    sleep(5);
    thpool_destroy(pool);
    //调用dao层接口查询
    DNSRecord *record = (DNSRecord *)malloc(sizeof(DNSRecord));
    memset(record, 0, sizeof(DNSRecord));
    for(int i = 0;i < len; i++){
        int ret = query_record(buffer[i].domin_name, buffer[i].record_type, record);
        if(ret == -1){
            LOG_ERROR("dao_query error");
        }
        //打印结果到LOG_INFO上
        // 先根据record_type判断是A还是AAAA
        char ip[INET6_ADDRSTRLEN] = {0};
        if(record->record_type == A){
            inet_ntop(AF_INET, record->record, ip, INET_ADDRSTRLEN);
        }else if(record->record_type == AAAA){
            inet_ntop(AF_INET6, record->record, ip, INET6_ADDRSTRLEN);
        }else{
            //CNAME or NS
            strcpy(ip, (char*)record->record);
        }
        LOG_INFO("domain:%s,record_type:%d,record:%s",record->domin_name,record->record_type,ip);
    }
    //释放内存
    free(buffer);
    free(record);
    destroy_dao();
    close_log();
    return 0;
}