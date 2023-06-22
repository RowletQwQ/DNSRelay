// Purpose: Header file for linked_list.c
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>
#include "datatype.h"

// 包含首部和尾部节点的结构体
struct list_ops_unit {
    struct linked_list_node *head;
    struct linked_list_node *tail;
};

// 双向链表节点
struct linked_list_node {
    struct linked_list_node *prev; // 指向前一个节点的指针
    struct linked_list_node *next; // 指向后一个节点的指针
    int32 data_len; // 数据长度
    int8 *data; // 数据
};

/*
    双向链表相关操作
*/

// 1.创建双向链表, 得到包含伪首部和伪尾部节点的结构体
struct list_ops_unit linked_list_create();

// 2.插入数据到双向链表首部, 返回值: 1-成功, 0-失败, 插入到链表首部
int32 linked_list_insert_head(struct linked_list_node *head, int8 *data, int32 data_len);

// 3.插入数据到双向链表尾部, 返回值: 1-成功, 0-失败, 插入到链表尾部
int32 linked_list_insert_tail(struct linked_list_node *head, int8 *data, int32 data_len);

// 4.删除双向链表的头节点, 返回值: 返回删除的节点, NULL表示删除失败
struct linked_list_node *linked_list_delete_head(struct linked_list_node *head);

// 5.删除双向链表的尾节点, 返回值: 返回删除的节点, NULL表示删除失败
struct linked_list_node *linked_list_delete_tail(struct linked_list_node *tail);

#endif 