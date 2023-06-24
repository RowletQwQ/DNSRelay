// 数据库

#ifndef DB_H
#define DB_H

#include "datatype.h"

#define DB_NAME "dns_relay.db"
#define TABLE_NAME "domin_table"
#define SUCCESS 1
#define FAIL 0

// 返回的dto数据
struct record_dto {
    uint16 record_dto;// 记录类型
    byte record[256]; // 记录数据
};

// 定义记录类型
#define A 1
#define AAAA 28
#define CNAME 5
#define NS 2
#define MX 15
#define TXT 16

// 存放域名信息的结构体, 对应表结构
struct domin_table_data {
    char domin_name[256];//域名
    uint16 record_type;// 记录类型
    byte record[256]; // 记录数据
    int64 expire_time; // 过期时间
};

/**
 * @brief 初始化数据库
 * 
 * @return int32 
 */
int32 init_db();

/**
 * @brief 根据域名查询, 结果为NULL表示查询失败
 * 
 * @param domin_name 域名
 * @return struct record_dto* 查询到的对应结果
 */
struct record_dto *query_by_domin_name(const char *domin_name);

// 3.插入一条域名信息
/**
 * @brief 插入一条域名信息, 如果已经存在, 则更新
 * @example
 * insert_domin_info("www.baidu.com", A, "220.181.38.150", 3600);
 * 
 * @param domin_name 域名
 * @param record_type 记录类型
 * @param record 记录数据
 * @param ttl Time To Live, 生存时间
 * @return int32 
 */
int32 insert_domin_info(const char *domin_name, uint16 record_type, byte record[256], int32 ttl);


#endif