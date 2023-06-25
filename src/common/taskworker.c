#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
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
    }
}


void taskworker(struct task * task_){
    
    printf("taskworker!!!\n");
    
    // 1.解析报文和查询报文
    int query_state = link_query_reqs(task_);

    // 依次打印message
    printf("taskworker: parse_to_reqs success\n");
    
    if(query_state == QUERY_FAIL){
        // 快速响应
        printf("query fail\n");
        
        // 只更新message
        int offset = task_->m_len;
        int res = throw_to_dns(task_);
        if(res == -1){
            printf("throw_to_dns failed!\n");
        }else{
            // 获取大小
            update_db(task_,offset); // 更新数据库
        }
    }else{
        // 查询成功
        parse_to_dnsres(task_);
        
        // 更新响应包
        printf("\nsento_client\n");
        // 发送响应包
        printf("%d\n",send_to_client(task_->message,task_->m_len,(struct sockaddr *)task_->addr,task_->sock_len));
    };
    
    
    // 释放资源
    task_free(task_);
    return;
}

// 创建完整链式请求 查询
int link_query_reqs(struct task * task_){
    // 解析报文头
    struct dns_header *dnshdr = (struct dns_header *)task_->message;
    // 解析请求数目
    task_->req_num = ntohs(dnshdr->qdcount);


    printf("req_num %d\n",task_->req_num);
    
    // 创建链表
    task_->reqs = linked_list_create();
    
    // 解析请求
    char * buf = task_->message;

    for(int i = 0,offset = 12;i < ntohs(dnshdr->qdcount);i++){    
        struct req temp_req;
        // 提供空间
        temp_req.req_domain = (char *)malloc(256);
        struct req req_;
        req_.domain_pointer = (int16) offset; //正常请求 初始化偏移值
        
        // 偏移增加 指向message中的消息
        offset += parse_to_req(buf + offset,&req_,(const char *)task_->message);
        // 查询该记录 , 填充 , 记录resp的原本名字作为CNAME域名

        int resp_type = query_req(&req_); 
        
        // 记录域名 可能作为下一个CNAME查询
        memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
        temp_req.domain_len = req_.rdata_len;
        // 格式化该请求
        parse_to_rdata(&req_);
        // 添加到链表中
        if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
            printf("Insert req failed!\n");
            exit(1);
        }
        int circle = 0;
        // 设置最大次数，链数目
        while(1){
            if(circle > MAX_REQLINK_NUM){
                return -1;
            }
            // 如果查询不到或者查询到了目标类型
            if(resp_type == -1 ){
                // 查询失败
                return -1;
            }else if(resp_type == req_.qtype){
                // 查询成功
                printf("query success\n");
                break;
            }else if(resp_type == 5){
                circle++;
                // CNAME
                printf("CNAME!!!\n");
                // 域名设置
                
                // 重新申请内存 在删除req时释放
                req_.req_domain = (char *)malloc(256);
                memcpy(req_.req_domain,temp_req.req_domain,temp_req.domain_len);
                req_.domain_len = temp_req.domain_len;
                // 指向前一个 的负数
                req_.domain_pointer = -req_.rdata_len;

                // 查询这个新的请求
                resp_type = query_req(&req_);
                
                
                printf(" ==%d==\n",resp_type);
                // 再次记录下
                memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
                temp_req.domain_len = req_.rdata_len;
                
                //格式化
                parse_to_rdata(&req_);

                // 添加到链表中
                if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
                    printf("Insert req failed!\n");
                    exit(1);
                }
                task_->req_num++;
            }else{
                // 查询出错
                return -1;
            }
        }
        // 释放temp——req空间
        free(temp_req.req_domain);
    }
    return 0;
}


void task_free(struct task * task_){
    // 释放资源

    // 1.释放message
    if(task_->message != NULL){
        free(task_->message);
    }
    // 2.释放地址
    struct sockaddr* addr = (struct sockaddr *)task_->addr;
    
    if(addr != NULL){
        free(addr);
    }

    // 3.释放task
    free(task_);
    return;
}

// 返回响应类型
int query_req(struct req * req_){
    
    // 创建socket
    
    // 设置超时
     
    // 随机数 假装查询
    
    if(rand() % 2 == 0){
        
        req_->rtype = 5;
        req_->rclass = 1;
        req_->ttl = 234;
        req_->rdata_len = 21;
        req_->rdata = (char *)malloc(sizeof(char) * 256);
        strcpy(req_->rdata,"www.baidu.baifen.com");
        return 5;
    }else{
        // 查询成功
        req_->rtype = req_->qtype;
        req_->rclass = 1;
        req_->ttl = 234;
        req_->rdata_len = 14;
        req_->rdata = (char *)malloc(sizeof(char) * 256);
        strcpy(req_->rdata,"110.114.117.6");
        return req_->qtype;
    }

    // 格式化r_data
    parse_to_rdata(req_);
    
    return QUERY_FAIL;
}

int update_db(struct task * task_,int offset){
    struct dns_header *dnshdr = (struct dns_header *)task_->message;
    int ancount = ntohs(dnshdr->ancount);
    
    // 创建缓存区
    struct req req_ ;
    
    // 依次解析answer
    for(int i = 0;i < ancount;i++){
        
        // 自己申请,一段空间存放，其他的直接引用    
        // 解析answer
        offset += parse_to_data(task_->message + offset,&req_,task_->message);
        
        // 添加到缓存
        printf("======debug======\n");
        // 打印req_
        printf("req_ domain_name %s\n",req_.req_domain);
        
        printf("req_ rdata %s\n",req_.rdata);
        
        printf("req_ rdata_len %d\n",req_.rdata_len);
        
        printf("req_ ttl %d\n",req_.ttl);
        
        printf("req_ type %d\n",req_.rtype);

        // 将这些数据写入文件
        // 写入文件
        
        // FILE *fp = fopen("db.txt","a+");
        // if(fp == NULL){
        //     printf("open file failed!\n");
        //     exit(1);
        // }

        // // 写入文件
        // fprintf(fp,"domain:%s rtype:%d ttl:%d rdata:%s\n",req_.req_domain,req_.rtype,req_.ttl,req_.rdata);
        
    }


}


int throw_to_dns (struct task* task_){
    
    // 可以审查下资质

    // 发送数据到dns服务器
    int state = talk_to_dns(task_->message,task_->m_len);
    // 依次打印message
    
    if(state == -1 || state == 0){
        // 超时或者查询失败
        printf("throw_to_dns: talk_to_dns fail\n");
        return -1;
    }else{
        // 查询成功
        // 检查资质
        task_->m_len = state;
        printf("throw_to_dns: talk_to_dns success\n");
    }
    // 给客户端发送过去
    send_to_client(task_->message,task_->m_len,(struct sockaddr *)task_->addr,task_->sock_len);
    return 0;
}