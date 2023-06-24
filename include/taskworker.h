#include "datatype.h"
#include "linked_list.h"

// 任务相关结构
#ifndef _TASKWORKER_H_
#define _TASKWORKER_H_


struct task{
    char * addr; // 任务地址
    int sock_len; // 套接字描述符
    char* message; // 报文
    int m_len; // 报文长度
    int req_num; // 请求个数
    struct list_ops_unit reqs; //链表指针
};

struct req{
    char* req_domain; //请求域名
    int16 domain_pointer; //域名指针
    int8 domain_len; //最大255
    int8 qtype; //请求类型  
    // 响应部分
    int8 rtype; //响应类型
    char * rdata; //响应数据
    int16 rdata_len; //响应数据长度
};
/**
 * @brief 处理一个任务池中的任务
 * @param task_ 处理好的任务
 */
void taskworker(struct task * task_);

/**
 * @brief 释放一个任务
 * @param task_ 待释放任务指针
 */
void task_free(struct task * task_);

/**
 * @brief 查询IP地址，给出查询结果
 * @param task_ 待释放的任务指针
 */
void task_query(struct task * task_);


/**
 * @brief 监听任务池，有任务则处理 需要多线程处理
 */
void taskmanager();
#endif