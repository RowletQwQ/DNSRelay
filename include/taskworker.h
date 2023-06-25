#include "datatype.h"
#include "linked_list.h"

// 任务相关结构
#ifndef _TASKWORKER_H_
#define _TASKWORKER_H_


#define QUERY_FAIL -1
#define MAX_REQLINK_NUM 5

struct task{
    char * addr; // 任务地址
    int sock_len; // 套接字描述符
    char* message; // 报文
    int m_len; // 报文长度
    int req_num; // 请求个数
    struct list_ops_unit reqs; //链表指针
};

// 一个请求可以有多个响应
struct req{
    char* req_domain; //请求域名
    int16 domain_pointer; //域名指针
    int8 domain_len; //最大255
    int8 qtype; //请求类型  
    int16 qclass; //请求类

    int8 rtype; //响应类型
    int16 rclass; //请求类
    char * rdata; //响应数据
    int16 rdata_len; //响应数据长度
    int32 ttl; //生存时间
};

struct resp{
    int8 rtype; //响应类型
    char * rdata; //响应数据
    int16 rdata_len; //响应数据长度
    int32 ttl; //生存时间
    
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

/**
 * @brief 肩负查询工作，查询缓存和数据库
 * 
 * @param req_ 待查询的请求
 */
int query_req(struct req * req_);


/**
 * @brief 把该任务交给DNS处理
 * @param task_ 待查询的任务，原始报文在其中
 * @return 0代表成功 -1代表失败
 */

int throw_to_dns (struct task* task_);

/**
 * @brief 在本地查询数据 填充到req里面
 * @param req_ 带查询的请求
 * @return 查询到的类型 -1代表查询失败
 */
int query_req(struct req * req_);

/**
 * @brief 链式查询请求
 * @param task_ 待查询的任务，原始报文在其中
 * @return 查询状态 -1代表查询失败 或者是最后的查询类型
 */
int link_query_reqs(struct task * task_);



/**
 * @brief 更新数据库和缓存
 * 
 * @param buf 报文数据 
 * @param str 常用字符串域名
 * @return int 转换成功的数据量
 */
int update_db(struct task * task_,int offset);
#endif