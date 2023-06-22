// File: trie.h, 字典树头文件
#ifndef _TRIE_H_
#define _TRIE_H_

#include <stddef.h>
#include "datatype.h"

// 字符集大小 26个字母 + 10个数字 + 2个特殊字符
#define CHARSET_SIZE 38 // 26 + 10 + 2
#define MAX_CACHE_SIZE 1024 // 最大缓存数量
#define IP_TYPE_IPV4 0
#define IP_TYPE_IPV6 1

// ip信息节点
struct ip_info {
    int32 query_cnt; // 查询次数
    uint16 ip_type;
    uint8 ip[16]; // IPv4地址   4字节
                  // IPv6地址   16字节
};

// 字典树节点
struct trie_node {
    struct trie_node *next[CHARSET_SIZE]; // 指向下一个节点的指针数组
    int32 through_cnt; // 经过该节点的字符串数量
    struct ip_info *ip_info; // ip信息, 只在一个字符串的最后一个字符节点上有值
};

/*
    字典树相关操作
*/

// 1.创建字典树, 返回值: 字典树根节点
struct trie_node *trie_create();

// 2.插入字符串(key为域名, value为ip地址), 返回值: 1-成功, 0-失败
int32 trie_insert(struct trie_node *root, int8 *key_domin_name, uint16 ip_type, uint8 value_ip[16]);

// 3.删除一个对应域名, 返回值: 1-成功, 0-失败
int32 trie_delete(struct trie_node *root, int8 *key_domin_name);

// 4.查找一个域名对应的ip地址, 返回值: ip地址, 为NULL表示查找失败
struct ip_info *trie_search(struct trie_node *root, int8 *key_domin_name);


#endif