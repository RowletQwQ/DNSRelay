//数据库交互访问层
#include "datatype.h"

#ifndef DAO_H
#define DAO_H

//常量
#define DAO_SUCCESS 0
#define DAO_FAILURE -1

//声明DNS记录的数据结构
typedef struct DNSRecord
{
    char domin_name[256];//域名
    uint16 record_type;// 记录类型
    int64 expire_time; // 过期时间
    uint16 record_len; // 记录长度
    byte record[256]; // 记录数据
} DNSRecord;

// 以下是DNS记录的相关数据操作,为外部提供一个透明的接口,隐藏内部的缓存和数据库的实现细节
// 增加一条记录
/**
 * @brief 增加一条记录
 * 
 * @param record 记录长度
 * @return int 成功返回0,失败返回-1
 */
int add_record(DNSRecord *record);

// 删除一条域名对应的所有记录
/**
 * @brief 删除一条域名对应的所有记录
 * 
 * @param domin_name 域名
 * @param record_type 记录类型
 * @return int 成功返回0,失败返回-1
 */
int delete_record(const char *domin_name);

// 删除一条域名对应的指定类型的记录
/**
 * @brief 删除一条域名对应的指定类型的记录
 * 
 * @param domin_name 域名
 * @param record_type 记录类型
 * @return int 成功返回0,失败返回-1
 */
int delete_record_by_type(const char *domin_name, uint16 record_type);

// 查询一条域名对应的所有记录
/**
 * @brief 查询一条域名对应的所有记录
 * 
 * @param domin_name 域名
 * @param record_type 记录类型
 * @param record 记录
 * @return int 成功返回长度,失败返回0
 */
int query_record(const char *domin_name, uint16 record_type, DNSRecord *record);



#endif