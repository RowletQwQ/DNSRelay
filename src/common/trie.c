#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

static int32 trans_char_to_index(int8 c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if (c >= '0' && c <= '9') {
        return c - '0' + 26;
    } else if (c == '.') {
        return 36;
    } else if (c == '-') {
        return 37;
    } else {
        return -1; // 表示不合法
    }
}

static struct trie_node *new_trie_node() {
    struct trie_node *node = (struct trie_node *)malloc(sizeof(struct trie_node));
    memset(node->next, 0, sizeof(node->next));
    node->through_cnt = 0;
    node->ip_info = NULL;
    node->list_node = NULL;
    return node;
}

static int32 cache_num(struct trie_node *root) {
    int32 num = 0;
    for (int i = 0; i < CHARSET_SIZE; i++) {
        if (root->next[i] != NULL) {
            num += root->next[i]->through_cnt;
        }
    }
    return num;
}


// 1.创建字典树, 返回值: 字典树根节点
struct trie_node *trie_create() {
    struct trie_node *root = (struct trie_node *)malloc(sizeof(struct trie_node));
    memset(root->next, 0, sizeof(root->next));
    root->through_cnt = 0;
    root->ip_info = NULL;
    root->list_node = NULL;

    ops_unit = linked_list_create(); // 链表操作单元
    return root;
}

// 2.插入字符串(key为域名, value为ip地址), 返回值: 1-成功, 0-失败
int32 trie_insert(struct trie_node *root, int8 *key_domin_name, uint16 ip_type, uint8 ip[16]) {
    // 如果缓存数量已经达到上限, 就需要先删除链表中最后一个节点
    if (cache_num(root) == MAX_CACHE_SIZE) {
        struct linked_list_node *last_node = linked_list_get_tail(ops_unit);
        if (last_node == NULL) {
            return 0;
        }
        if (sizeof(struct put_list_data) != last_node->data_len) {
            return 0;
        }
        struct put_list_data *put_list_data = (struct put_list_data *)last_node->data;
        if (put_list_data == NULL) {
            return 0;
        }
        if (trie_delete(root, put_list_data->domin_name) == 0) {
            return 0;
        }
    }


    struct trie_node *cur = root;
    // 遍历域名
    while (*key_domin_name != '\0') {
        int8 c = *key_domin_name;
        int32 c_index = trans_char_to_index(c);
        if (c_index == -1) {
            return 0;
        }
        if (cur->next[c_index] != NULL) {
            cur = cur->next[c_index];
            cur->through_cnt++;
            continue;
        }
        // 否则新建一个字典树节点
        cur->next[c_index] = new_trie_node();
        cur = cur->next[c_index];
        cur->through_cnt++;  

        // key_domin_name指针后移
        key_domin_name++;
    }

    // 给当前节点添加ip信息 (因为当前节点是字符串的最后一个位置)
    struct ip_info *ip_info = (struct ip_info *)malloc(sizeof(struct ip_info));
    ip_info->ip_type = ip_type;
    memcpy(ip_info->ip, ip, 16);
    cur->ip_info = ip_info;

    // 添加放入链表的节点指针
    struct put_list_data *put_list_data = (struct put_list_data *)malloc(sizeof(struct put_list_data));
    put_list_data->query_cnt = 0; // 查询次数初始化为0
    put_list_data->domin_name = key_domin_name; 
    linked_list_insert_tail(ops_unit, (int8*)put_list_data, sizeof(struct put_list_data));
    cur->list_node = linked_list_get_tail(ops_unit);

    return 1;
}

// 3.删除一个对应域名, 返回值: 1-成功, 0-失败
int32 trie_delete(struct trie_node *root, int8 *key_domin_name) {
    struct trie_node *cur = root;
    // 遍历域名
    while (*key_domin_name != '\0') {
        int8 c = *key_domin_name;
        int32 index = trans_char_to_index(c);
        if (index == -1) {
            return 0;
        }
        if (cur->next[index] == NULL) {
            return 0;
        }
        cur = cur->next[index];
        if (--cur->through_cnt == 0) {
            free_trie_node(cur);
            return 1;
        }
        // key_domin_name指针后移
        key_domin_name++;
    }
}

// 4.查找一个域名对应的ip地址, 返回值: ip地址, 为NULL表示查找失败
struct ip_info *trie_search(struct trie_node *root, int8 *key_domin_name) {
    struct trie_node *cur = root;
    // 遍历域名
    while (*key_domin_name != '\0') {
        int8 c = *key_domin_name;
        int32 index = trans_char_to_index(c);
        if (index == -1) {
            return NULL;
        }
        if (cur->next[index] == NULL) {
            return NULL;
        }
        cur = cur->next[index];
        // key_domin_name指针后移
        key_domin_name++;
    }

    // 将cur指向的节点的链表节点的查询次数加1
    if ((int32)sizeof(struct put_list_data) == cur->list_node->data_len) {
        struct put_list_data *put_list_data = (struct put_list_data *)cur->list_node->data;
        put_list_data->query_cnt++;
        cur->list_node->data = (int8*)put_list_data;
        // 然后将cur->list_node向前移动
        while (1) {
            if (cur->list_node->prev == ops_unit.head) {
                break;
            }
            struct put_list_data *pre_list_data = (struct pre_list_data *)cur->list_node->prev->data;
            if (put_list_data->query_cnt <= pre_list_data->query_cnt) {
                break;
            }
            // 交换两个节点的数据
            linked_list_swap_node(cur->list_node, cur->list_node->prev);
            cur->list_node = cur->list_node->prev;
        }
        return cur->ip_info;
    }
    return NULL;
}

// 5.递归释放字典树节点
void free_trie_node(struct trie_node *node) {
    if (node == NULL) {
        return;
    }
    for (int32 i = 0; i < CHARSET_SIZE; i++) {
        free_trie_node(node->next[i]);
    }

    // 删除ip信息
    if (node->ip_info != NULL) {
        free(node->ip_info);
    }
    // 删除链表节点
    if (node->list_node != NULL) {
        linked_list_free_node(node->list_node);
    }
    free(node);
}