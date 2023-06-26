#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dao.h"
#include "trie.h"
#include "db.h"
#include "logger.h"
#include "userfile.h"
#include "thpool.h"
//测试dao层工作是否正常
//测试通过读取userfile.txt文件实现,先读取userfile.txt
//调用dao层接口函数进行写入，随后进行查询

int main(){
    init_log("log.txt", LOG_LEVEL_DEBUG, 1, NULL);
    init_read_file("userfile.txt");
    init_dao();
    struct domin_table_data *buffer = NULL;
    int len = read_data(&buffer);
    printf("len:%d\n", len);
    //调用dao层接口写入
    for(int i = 0; i < len; i++){
        int ret = add_record(buffer[i].domain, buffer[i].record_type, buffer[i].record,256,30);
        if(ret == -1){
            LOG_ERROR("dao_insert error");
        }
    }
    //调用dao层接口查询
    DNSRecord *record = (DNSRecord *)malloc(sizeof(DNSRecord));
    memset(record, 0, sizeof(DNSRecord));
    for(int i = 0;i < len; i++){
        int ret = query_record(buffer[i].domain, buffer[i].record_type, record);
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
            strcpy(ip, record->record);
        }
        LOG_INFO("domain:%s,record_type:%d,record:%s",record->domain,record->record_type,ip);
    }
    //释放内存
    free(buffer);
    free(record);
    destroy_dao();
    return 0;
}