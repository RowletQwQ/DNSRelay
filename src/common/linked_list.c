#include "linked_list.h"

// 1.创建双向链表, 得到包含首部和尾部节点的结构体
struct list_ops_unit linked_list_create() {
    struct list_ops_unit list;
    list.head = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    list.tail = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    // 伪头部节点 
    list.head->next = list.tail;
    list.head->prev = NULL;
    list.head->data_len = 0;
    list.head->data = NULL;

    // 伪尾部节点
    list.tail->prev = list.head;
    list.tail->next = NULL;
    list.tail->data_len = 0;
    list.tail->data = NULL;
    
    return list;
}