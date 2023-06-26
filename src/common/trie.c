#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

// 一些静态函数
static struct list_ops_unit main_ops_unit;

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
    node->ops_unit = NULL;
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
    root->ops_unit = NULL;
    main_ops_unit = linked_list_create(); // 链表操作单元
    return root;
}

// 2.插入域名记录信息, 返回值: 1-成功, 0-失败
int32 trie_insert(struct trie_node *root, int8 *key_domin_name, uint16 record_type, uint16 record_len, byte record[256], int32 ttl) {
    int8 *domin_name = key_domin_name;
    
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
            // key_domin_name指针后移
            key_domin_name++;
            continue;
        }
        // 否则新建一个字典树节点
        cur->next[c_index] = new_trie_node();
        cur = cur->next[c_index];
        cur->through_cnt++; 

        // key_domin_name指针后移
        key_domin_name++;
    }

    // 先遍历之前的域名信息, 如果发现之前已经存有相同的类型, 则比对他们的过期时间
    if (cur->ops_unit != NULL) {
        linked_list_node *cur_node = cur->ops_unit->head->next;
        while (cur_node != cur->ops_unit->tail) {
            assert(cur_node->data_len == sizeof(struct linked_list_node));
            linked_list_node *data_ptr = (linked_list_node *)cur_node->data;
            assert(data_ptr->data_len == sizeof(struct record_info));
            struct record_info *record_info_ = (struct record_info *)data_ptr->data;
            // 如果发现record_info_的记录类型跟插入的类型一样
            if (record_info_->record_type == record_type) {
                int8 *tmp_domin_name = domin_name;
                struct trie_node *cur_tmp = root;
                while (*tmp_domin_name != '\0') {
                    int8 c = *tmp_domin_name;
                    int index = trans_char_to_index(c);
                    cur_tmp = cur_tmp->next[index];
                    cur_tmp->through_cnt--;
                }
                if (ttl + time(NULL) <= record_info_->expire_time) {
                    // 如果发现插入的过期时间比原来的过期时间还早, 则不执行插入
                    return SUCCESS;
                } else {
                    // 删除主链表中的节点
                    linked_list_delete_node(main_ops_unit, data_ptr);
                    linked_list_free_node(data_ptr);   

                    // 删除cur->ops_unit中节点
                    linked_list_delete_node(*cur->ops_unit, cur_node);
                    linked_list_free_node(cur_node);
                    break;
                }
            }
            cur_node = cur_node->next;
        }
    }


    // 此时cur指向最后一个节点, 此时需要将对应的ip地址信息插入到链表中
    struct record_info *record_info = (struct record_info *)malloc(sizeof(struct record_info));
    record_info->record_type = record_type; // 记录类型
    record_info->record_len = record_len; // 记录长度
    memcpy(record_info->record, record, record_len); //记录数据
    record_info->expire_time = time(NULL) + ttl; // 过期时间
    record_info->domin_name = domin_name; // 域名
    record_info->query_cnt = 0; // 查询次数

    // 插入主链表中, 并得到首部节点
    linked_list_insert_head(main_ops_unit, (char*)record_info, sizeof(struct record_info));
    struct linked_list_node *tail = linked_list_get_head(main_ops_unit);
     
    // 如果cur->ops_unit为空, 则新建一个链表操作单元
    if (cur->ops_unit == NULL) {
        cur->ops_unit = malloc(sizeof(struct list_ops_unit));
        *cur->ops_unit = linked_list_create();
    } 
    // 此处使用了指针嵌套
    if (linked_list_insert_head(*cur->ops_unit, (char*)tail, sizeof(struct linked_list_node)) == FAIL) {
        return FAIL;
    }


    // 如果缓存数量已经达到上限, 就需要先删除链表中最后一个节点
    while (cache_num(root) > MAX_CACHE_SIZE) {
        linked_list_node *last = linked_list_delete_tail(main_ops_unit);
        if (sizeof(struct record_info) == last->data_len) {
            struct record_info *record_info = (struct record_info *)last->data;
            trie_delete(root, record_info->domin_name, record_info->record_type);
        } else {
            return FAIL;
        }
    }

    return SUCCESS;
}

// 3.删除一个对应域名, 返回值: 1-成功, 0-失败
int32 trie_delete(struct trie_node *root, int8 *key_domin_name, uint16 record_type) {
    struct trie_node *cur = root;
    struct trie_node *pre = root;
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
            pre->next[index] = NULL;
            free_trie_node(cur);
            return 1;
        }
        pre = cur;
        // key_domin_name指针后移
        key_domin_name++;
    }

    // 然后如果发现没有节点的经过次数变为0, 则说明此时cur下面的指针数组大小是一个以上, 此时需要遍历这个指针数组, 将对应类型删除, 并释放
    delete_record_list_node(*cur->ops_unit, record_type);
    return 1;
}

int32 delete_record_list_node(struct list_ops_unit ops_unit, uint16 record_type) {
    struct linked_list_node *cur = ops_unit.head;
    while (cur != NULL && cur != ops_unit.tail) {
        if (cur->data_len != sizeof(struct linked_list_node)) {
            cur = cur->next;
            continue;
        }
        struct linked_list_node *node = (struct linked_list_node*)cur->data;
        struct record_info *record_info = (struct record_info *)node->data;
        if (record_info->record_type == record_type) {
            linked_list_delete_node(main_ops_unit, node);
            linked_list_free_node(node); // 释放链表节点 
            linked_list_delete_node(ops_unit, cur);
            free(cur);
            return SUCCESS;
        }
        cur = cur->next;
    }
    return FAIL;
}

// 4.查找一个域名对应的记录数据, 返回值: 记录数据, 为NULL表示查找失败
struct record_info *trie_search(struct trie_node *root, int8 *key_domin_name, uint16 record_type) {
    struct trie_node *cur = root;
    int8 *domin_name = key_domin_name;
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

    // 利用cur->ops_unit遍历链表, 找到对应的记录数据
    struct linked_list_node *cur_node = cur->ops_unit->head;
    while (cur_node != NULL && cur_node != cur->ops_unit->tail) {
        if (cur_node->data_len != sizeof(struct linked_list_node)) {
            cur_node = cur_node->next;
            continue;
        }
        struct linked_list_node *node = (struct linked_list_node*)cur_node->data;
        struct record_info *record_info = (struct record_info *)node->data;
        if (record_info->record_type == record_type) {
            // 如果发现这个记录已经过期, 则删除这个记录
            if (record_info->expire_time == -1 && record_info->expire_time <= time(NULL)) {
                printf("record is expire, delete it\n");
                printf("domin_name: %s, record_type: %d\n", domin_name, record_type);
                trie_delete(root, domin_name, record_type);
                return NULL;
            }

            // 当找到这个数据后, 让记录的查询数+1
            record_info->query_cnt++;
            // while (node->prev != NULL && node->prev != main_ops_unit.head) {
            //     struct linked_list_node *prev_node = node->prev;
            //     struct record_info *prev_record_info = (struct record_info *)prev_node->data;
            //     if (prev_record_info->query_cnt >= record_info->query_cnt) {
            //         break;
            //     }
            //     // 交换两个节点的数据
            //     linked_list_swap_node(node, prev_node);
            //     node = node->prev;
            // }
            // 查询后直接把这个节点移动到链表头部, 利用LRU算法
            linked_list_move_node_to_head(main_ops_unit, node);
            return record_info;
        }
        cur_node = cur_node->next;
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

    // 释放链表节点
    if (node->ops_unit != NULL) {
        linked_list_free(*node->ops_unit);
    }

    free(node);
}