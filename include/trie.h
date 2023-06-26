// File: trie.h, 字典树头文件
#ifndef _TRIE_H_
#define _TRIE_H_

#include <stddef.h>
#include "datatype.h"
#include "linked_list.h"

// 字符集大小 26个字母 + 10个数字 + 2个特殊字符
#define CHARSET_SIZE 38 // 26 + 10 + 2
#define MAX_CACHE_SIZE 1024 // 最大缓存数量
#define IP_TYPE_IPV4 0
#define IP_TYPE_IPV6 1
#define SUCCESS 1
#define FAIL 0

// 定义记录类型
#define A 1
#define AAAA 28
#define CNAME 5
#define NS 2
#define MX 15
#define TXT 16


// 记录信息节点
struct record_info {
    uint16 record_type; // 记录类型
    int64 expire_time; // 每条记录会对应一个过期时间
    int8 *domin_name; // 对应的域名
    int32 query_cnt; // 该条记录的查询次数
    uint16 record_len; // 记录长度
    byte record[256]; // 记录数据
};

// 字典树节点
struct trie_node {
    struct trie_node *next[CHARSET_SIZE]; // 指向下一个节点的指针数组
    int32 through_cnt; // 经过该节点的字符串数量
    // struct record_info **record_info_array; // 记录信息, 指向一个指针数组
    struct list_ops_unit *ops_unit; // 将上述指针数组用链表代替
};



/*
    字典树相关操作
*/

/**
 * @brief 创建字典树
 * 
 * @return struct trie_node* 返回字典树根节点 
 */
struct trie_node *trie_create();

/**
 * @brief 插入一个域名对应的ip地址, 返回值: 1-成功, 0-失败
 * 
 * @param root 字典树根节点
 * @param key_domin_name 根域名
 * @param record_type 记录类型
 * @param record 记录数据
 * @param ttl 过期时间
 * @return int32 1-成功, 0-失败
 */
int32 trie_insert(struct trie_node *root, int8 *key_domin_name, uint16 record_type, uint16 record_len, byte record[256], int32 ttl);

/**
 * @brief 删除一个对应域名, 返回值: 1-成功, 0-失败
 * 
 * @param root 字典树根节点
 * @param key_domin_name 根域名
 * @return int32 1-成功, 0-失败
 */
int32 trie_delete(struct trie_node *root, int8 *key_domin_name, uint16 record_type);

/**
 * @brief 根据记录类型删除一个记录, 返回值: 1-成功, 0-失败
 * 
 * @param ops_unit 操作单元
 * @param record_type 记录类型
*/
int32 delete_record_list_node(struct list_ops_unit ops_unit, uint16 record_type);

/**
 * @brief 查找一个域名对应的ip地址, 返回值: ip地址, 为NULL表示查找失败
 * 
 * @param root 字典树根节点
 * @param key_domin_name 根域名
 * @param record_type 记录类型
 * @return struct record_info* 记录信息, 为NULL表示查找失败
 */
struct record_info *trie_search(struct trie_node *root, int8 *key_domin_name, uint16 record_type);

/**
 * @brief 递归删除所有节点
 * 
 * @param node 字典树节点
 */
void free_trie_node(struct trie_node *node);




#endif