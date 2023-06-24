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
#define SUCESS 1
#define FAIL 0

// ip信息节点
struct ip_info {
    uint16 ip_type;
    uint8 ip[16]; // IPv4地址 4字节 , IPv6地址   16字节
};

// 放入双向链表的数据, 包括域名和查询次数, 查询次数用于LRU算法, 域名用于删除字典树节点
struct put_list_data {
    int32 query_cnt;
    int8 *domin_name;
    int64 expire_time; // 过期时间
};

// 字典树节点
struct trie_node {
    struct trie_node *next[CHARSET_SIZE]; // 指向下一个节点的指针数组
    int32 through_cnt; // 经过该节点的字符串数量
    struct ip_info *ip_info; // ip信息, 只在一个字符串的最后一个字符节点上有值
    struct linked_list_node *list_node; // 指向一个链表节点
};


// 一些静态函数
static struct list_ops_unit ops_unit;

/**
 * @brief 将字符转换为字典树节点的索引
 * 
 * @param c 字符
 * @return int32 字典树节点的索引 
 */
static int32 trans_char_to_index(int8 c);

/**
 * @brief 创建一个新的字典树节点
 * 
 * @return struct trie_node* 返回新的字典树节点
 */
static struct trie_node *new_trie_node();

/**
 * @brief 计算字典树中节点的数量
 * 
 * @param root 字典树节点
 * @return int32 返回总数量
 */
static int32 cache_num(struct trie_node *root);

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
 * @param ip_type ip类型
 * @param ip ip地址
 * @return int32 1-成功, 0-失败
 */
int32 trie_insert(struct trie_node *root, int8 *key_domin_name, uint16 ip_type, uint8 ip[16], int32 ttl);

/**
 * @brief 删除一个对应域名, 返回值: 1-成功, 0-失败
 * 
 * @param root 字典树根节点
 * @param key_domin_name 根域名
 * @return int32 1-成功, 0-失败
 */
int32 trie_delete(struct trie_node *root, int8 *key_domin_name);

/**
 * @brief 查找一个域名对应的ip地址, 返回值: ip地址, 为NULL表示查找失败
 * 
 * @param root 字典树根节点
 * @param key_domin_name 根域名
 * @return struct ip_info* ip地址, 为NULL表示查找失败
 */
struct ip_info *trie_search(struct trie_node *root, int8 *key_domin_name);

/**
 * @brief 递归删除所有节点
 * 
 * @param node 字典树节点
 */
void free_trie_node(struct trie_node *node);




#endif