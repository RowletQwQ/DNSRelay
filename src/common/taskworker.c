#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "taskworker.h"
#include "socket.h"
#include "parsedata.h"

extern struct list_ops_unit task_pool;
extern int task_free_flag;
// 任务池
void taskmanager(){
    struct linked_list_node * task_node = NULL;
    Sleep(100);
    while(1){
        if((task_node = linked_list_delete_head(task_pool)) != NULL){
            
            struct task* task_ = (struct task *)task_node->data;
            
            // 打印task_的内容
            printf("\n message_len %d\n",task_->m_len);

            for(int i = 0;i < task_->m_len;i++){
                printf("%02x ",task_->message[i]);
            }
            
            // 交给线程池处理
            taskworker((struct task *)task_node->data);
        }else{
            Sleep(100);
        }
        // 从任务池中取出任务
        // if(task_free_flag == 1){    
        //     task_free_flag = 0;
            
        //     task_free_flag = 1;
        // }
        
    }
}


void taskworker(struct task * task_){
    // 1.解析报文
    parse_to_reqs(task_);
    printf("taskworker: parse_to_reqs success\n");
    
    // 打印查询的数据
    // struct linked_list_node* head = task_->reqs.head;
    
    // 这里不太清楚为什么不能递增找到
    int i = 0;
    struct linked_list_node* node_ = linked_list_get_head(task_->reqs);
    struct req* req_;
    if(node_ != NULL){
        req_ = (struct req*)node_->data;
        printf("%s\n",req_->req_domain);
        req_->rtype = req_->qtype;
        req_->rdata = "123.0.2.123";
        req_->rdata_len = 4;
    }else{
        return;
    }
    
    // 查询数据 填充reqs
        
        // 2.查询缓存

        // 3.查询数据库

        // 4.查询远程服务器

    parse_to_dnsres(task_);

    // 更新响应包
    printf("\nsento_client\n");
    
    // 打印task的地址


    // 发送响应包
    printf("%d\n",send_to_client(task_->message,task_->m_len,(struct sockaddr *)task_->addr,task_->sock_len));
    // 8.释放资源
    
    // task_free(task_);
    // 9.返回
    return;
}
void task_free(struct task * task_){
    // 释放资源

    // 1.释放message
    free(task_->message);
    // 2.释放task
    free(task_);
    // 3.释放地址
    struct sockaddr* addr = (struct sockaddr *)task_->addr;
    free(addr);
    return;
}

void task_query(struct task * task_){
    for(int i = 0; i < task_->req_num; i++){
        
        // 1.查询缓存
        // 2.查询数据库
        // 3.查询远程服务器
    }
}