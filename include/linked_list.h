// Purpose: Header file for linked_list.c
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>
#include "datatype.h"

#define SUCESS 1
#define FAIL 0

// 包含首部和尾部节点的结构体
struct list_ops_unit {
    struct linked_list_node *head;
    struct linked_list_node *tail;
};
typedef struct list_ops_unit list_ops_unit;
// 双向链表节点
struct linked_list_node {
    struct linked_list_node *prev; // 指向前一个节点的指针
    struct linked_list_node *next; // 指向后一个节点的指针
    int32 data_len; // 数据长度
    int8 *data; // 数据
};
typedef struct linked_list_node linked_list_node;

/*
    双向链表相关操作
*/


/**
 * @brief 创建双向链表, 得到包含伪首部和伪尾部节点的结构体
 * 
 * @return struct list_ops_unit 返回包含伪首部和伪尾部节点的结构体
 */
struct list_ops_unit linked_list_create();

/**
 * @brief 插入数据到双向链表头部, 返回值: 1-成功, 0-失败, 插入到链表头部
 * 
 * @param ops_unit 
 * @param data 
 * @param data_len 
 * @return int32 
 */
int32 linked_list_insert_head(struct list_ops_unit ops_unit, int8 *data, int32 data_len);

/**
 * @brief 插入数据到双向链表尾部, 返回值: 1-成功, 0-失败, 插入到链表尾部
 * 
 * @param ops_unit 双向链表
 * @param data 数据
 * @param data_len 数据长度
 * @return int32 是否插入成功，1-成功，0-失败
 */
int32 linked_list_insert_tail(struct list_ops_unit ops_unit, int8 *data, int32 data_len);

/**
 * @brief 删除双向链表的头节点, 返回值: 返回删除的节点, NULL表示删除失败
 * 
 * @param ops_unit 双向链表
 * @return struct linked_list_node* 返回删除的节点, NULL表示删除失败
 */ 
struct linked_list_node *linked_list_delete_head(struct list_ops_unit ops_unit);

/**
 * @brief 删除双向链表的尾节点, 返回值: 返回删除的节点, NULL表示删除失败
 * 
 * @param ops_unit 双向链表
 * @return struct linked_list_node* 返回删除的节点, NULL表示删除失败 
 */
struct linked_list_node *linked_list_delete_tail(struct list_ops_unit ops_unit);

/**
 * @brief 返回第一个节点
 * 
 * @param ops_unit 双向链表
 * @return struct linked_list_node* 返回第一个节点, NULL表示没有节点
 */
struct linked_list_node *linked_list_get_head(struct list_ops_unit ops_unit);

/**
 * @brief 返回最后一个节点
 * 
 * @param ops_unit 双向链表
 * @return struct linked_list_node* 返回最后一个节点, NULL表示没有节点
 */
struct linked_list_node *linked_list_get_tail(struct list_ops_unit ops_unit);

/**
 * @brief 交换两个节点(保证两个节点相邻)
 * 
 * @param node1 节点1
 * @param node2 节点2
 */
void linked_list_swap_node(struct linked_list_node *node1, struct linked_list_node *node2);


/**
 * @brief 释放一个节点的内存
 * 
 * @param node 节点
 */
void linked_list_free_node(struct linked_list_node *node);

/**
 * @brief 释放双向链表的内存
 * 
 * @param ops_unit 双向链表
 */
void linked_list_free(struct list_ops_unit ops_unit);



#endif 