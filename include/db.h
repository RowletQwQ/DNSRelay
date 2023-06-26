// 数据库

#ifndef DB_H
#define DB_H

#include "datatype.h"

#define DB_NAME "dns_relay.db"
#define TABLE_NAME "domin_table"
#define SUCCESS 1
#define FAIL 0

// 定义记录类型
#define A 1
#define AAAA 28
#define CNAME 5
#define NS 2
#define MX 15
#define TXT 16


struct record_dto {
    byte record[256]; // 记录数据
    uint16 record_len; // 记录长度
    int64 expire_time; // 过期时间
};

// 存放域名信息的结构体, 对应表结构
struct domin_table_data {
    char domin_name[256];//域名
    uint16 record_type;// 记录类型
    byte record[256]; // 记录数据
    uint16 record_len; // 记录长度
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
struct record_dto *query_by_domin_name(const char *domin_name, uint16 record_type);

// 3.插入一条域名信息
/**
 * @brief 插入一条域名信息, 如果已经存在, 则更新
 * @example
 * insert_domin_info("www.baidu.com", A, []uint8{220, 181, 38, 150}, 3600);
 * 
 * @param domin_name 域名
 * @param record_type 记录类型
 * @param record 记录数据
 * @param record_len 记录长度
 * @param ttl Time To Live, 生存时间
 * @return int32 
 */
int32 insert_domin_info(const char *domin_name, uint16 record_type, byte record[256], uint16 record_len, int32 ttl);

// 4.查询所有域名信息
/**
 * @brief 查询所有域名信息
 * 
 * @param domin_table_data_array 存放域名信息的数组
 * @return int32 返回数组长度
 */ 
int32 query_all_dns_record(struct domin_table_data **domin_table_data_array);

#endif