#include "linked_list.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

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

// 2.插入数据到双向链表首部, 返回值: 1-成功, 0-失败, 插入到链表首部
int32 linked_list_insert_head(struct list_ops_unit ops_unit, int8 *data, int32 data_len) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return 0;
    }

    struct linked_list_node *node = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    node->data_len = data_len;
    node->data = (int8 *)malloc(data_len);
    memcpy(node->data, data, data_len);

    // 插入到链表首部
    node->next = ops_unit.head->next;
    node->prev = ops_unit.head;
    ops_unit.head->next->prev = node;
    ops_unit.head->next = node;


    return 1;
}

// 3.插入数据到双向链表尾部, 返回值: 1-成功, 0-失败, 插入到链表尾部
int32 linked_list_insert_tail(struct list_ops_unit ops_unit, int8 *data, int32 data_len) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return 0;
    }

    struct linked_list_node *node = (struct linked_list_node *)malloc(sizeof(struct linked_list_node));
    node->data_len = data_len;
    node->data = (int8 *)malloc(data_len);
    memcpy(node->data, data, data_len);

    // 插入到链表尾部
    node->next = ops_unit.tail;
    node->prev = ops_unit.tail->prev;
    ops_unit.tail->prev->next = node;
    ops_unit.tail->prev = node;

    return 1;
}

// 4.删除双向链表的头节点, 返回值: 返回删除的节点, NULL表示删除失败
struct linked_list_node *linked_list_delete_head(struct list_ops_unit ops_unit) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return NULL;
    }

    if (ops_unit.head->next == ops_unit.tail) {
        return NULL;
    }

    struct linked_list_node *node = ops_unit.head->next;
    ops_unit.head->next = node->next;
    node->next->prev = ops_unit.head;

    return node;
}

// 5.删除双向链表的尾节点, 返回值: 返回删除的节点, NULL表示删除失败
struct linked_list_node *linked_list_delete_tail(struct list_ops_unit ops_unit) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return NULL;
    }

    if (ops_unit.head->next == ops_unit.tail) {
        return NULL;
    }

    struct linked_list_node *node = ops_unit.tail->prev;
    ops_unit.tail->prev = node->prev;
    node->prev->next = ops_unit.tail;

    return node;
}

// 6.返回第一个节点, NULL表示没有节点
struct linked_list_node *linked_list_get_head(struct list_ops_unit ops_unit) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return NULL;
    }

    if (ops_unit.head->next == ops_unit.tail) {
        return NULL;
    }

    return ops_unit.head->next;
}

// 7.返回最后一个节点, NULL表示没有节点
struct linked_list_node *linked_list_get_tail(struct list_ops_unit ops_unit) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return NULL;
    }

    if (ops_unit.head->next == ops_unit.tail) {
        return NULL;
    }

    return ops_unit.tail->prev;
}

// 8.交换两个节点的位置
void linked_list_swap_node(struct linked_list_node *node1, struct linked_list_node *node2) {
    if (node1 == NULL || node2 == NULL) {
        return;
    }

    int8 *tmp_data = node1->data;
    int32 tmp_data_len = node1->data_len;
    node1->data = node2->data;
    node1->data_len = node2->data_len;
    node2->data = tmp_data;
    node2->data_len = tmp_data_len;
}

// 9.释放双向链表的节点
void linked_list_free_node(struct linked_list_node *node) {
    if (node == NULL) {
        return;
    }

    if (node->data != NULL) {
        free(node->data);
        node->data = NULL;
    }

    free(node);
    node = NULL;
}

// 10.释放双向链表的内存
void linked_list_free(struct list_ops_unit ops_unit) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return;
    }

    struct linked_list_node *node = ops_unit.head->next;
    while (node != ops_unit.tail) {
        struct linked_list_node *tmp = node;
        node = node->next;
        linked_list_free_node(tmp);
    }

    linked_list_free_node(ops_unit.head);
    linked_list_free_node(ops_unit.tail);
}

int32 linked_list_delete_node(struct list_ops_unit ops_unit, struct linked_list_node *node) {
    if (ops_unit.head == NULL || ops_unit.tail == NULL) {
        return 0;
    }

    if (node == NULL) {
        return 0;
    }

    if (node == ops_unit.head || node == ops_unit.tail) {
        return 0;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    return 1;
}