// 数据库

#ifndef DB_H
#define DB_H

#include "datatype.h"

#define DB_NAME "dns_relay.db"
#define TABLE_NAME "domin_table"
#define SUCCESS 1
#define FAIL 0

// 返回的dto数据
struct ip_dto {
    uint16 ip_type;
    uint8 ip[16];
};


// 存放域名信息的结构体, 对应表结构
struct domin_table_data {
    int8 *domin_name;
    uint16 ip_type;
    uint8 ip[16]; // IPv4地址 4字节 , IPv6地址   16字节
    int64 expire_time;
};

// 1.初始化数据库, 建表
int32 init_db();

// 2.根据域名查询, 结果为NULL表示查询失败
struct ip_dto *query_by_domin_name(int8 *domin_name);

// 3.插入一条域名信息
int32 insert_domin_info(int8 *domin_name, uint16 ip_type, uint8 ip[16], int32 ttl);


#endif