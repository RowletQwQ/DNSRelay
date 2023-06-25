#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "taskworker.h"
#include "socket.h"
#include "parsedata.h"

#include "trie.h"

#include "logger.h"
extern struct list_ops_unit task_pool;
// 字典树缓存
extern struct trie_node * trie_cache;

// 任务池
void taskmanager(void * arg){
    (void)arg;
    struct linked_list_node * task_node = NULL;
    
    Sleep(100);
    while(1){
        if((task_node = linked_list_delete_head(task_pool)) != NULL){
            struct task* task_ = (struct task *)task_node->data;
            // 交给线程池处理
            printf("taskmanager\n");
            taskworker((struct task *)task_node->data);
        }else{
            Sleep(100);
        }
    }
}


void taskworker(struct task * task_){
    LOG_INFO("%s taskworker start\n",task_->addr);
    
    // 1.解析报文和查询报文
    int query_state = link_query_reqs(task_);

    // 依次打印message
    if(query_state == QUERY_FAIL){
        // 快速响应
        LOG_INFO("QUERY_LINK FAIL\n");
        
        // 只更新message
        int offset = task_->m_len;
        int res = throw_to_dns(task_);
        
        if(res == -1){
            LOG_INFO("throw_to_dns FAIL\n");
        }else{
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

    LOG_INFO("req_num:%d\n",task_->req_num);
    
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
        
        int resp_type = query_req(&req_); 
        
        // 记录域名 可能作为下一个CNAME查询
        memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
        
        temp_req.domain_len = req_.rdata_len;
        // 格式化该请求
        parse_to_rdata(&req_);
        // 添加到链表中
        if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
            LOG_ERROR("Insert req %s failed!\n",req_.req_domain);
            return -1;
        }

        // 设置最大次数，链数目
        int circle = 0;
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
                LOG_INFO("query success!\n");
                break;
            }else if(resp_type == 5){
                circle++;
                // CNAME

                // 重新申请内存 在删除req时释放
                req_.req_domain = (char *)malloc(256);
                
                memcpy(req_.req_domain,temp_req.req_domain,temp_req.domain_len);
                req_.domain_len = temp_req.domain_len;
                
                // 指向前一个负数
                req_.domain_pointer = -req_.rdata_len;
                // 查询这个新的请求
                resp_type = query_req(&req_);
                LOG_INFO("QUERY TYPE:%d\n",resp_type);
                
                // 再次记录下
                memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
                temp_req.domain_len = req_.rdata_len;
                
                //格式化
                parse_to_rdata(&req_);

                // 添加到链表中
                if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
                    LOG_ERROR("Insert req %s failed!\n",req_.req_domain);
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
    // 随机数 假装查询
    struct record_info * query_info =  trie_search(trie_cache,req_->req_domain,req_->qtype);
    req_->rdata = (char *)malloc(sizeof(char) * 256);
    if(query_info == NULL){
        // 查询失败
        return -1;
    }else{
        // 查询成功
        req_->rtype = query_info->record_type;
        req_->ttl = query_info->expire_time - time(NULL);
        req_->rdata_len = query_info->record_len;
        memcpy(req_->rdata,query_info->record,query_info->record_len);
        return query_info->record_type;
    }
    
    
    // if(rand() % 2 == 0){    
    //     req_->rtype = 5;
    //     req_->rclass = 1;
    //     req_->ttl = 234;
    //     req_->rdata_len = 21;
    //     req_->rdata = (char *)malloc(sizeof(char) * 256);
    //     strcpy(req_->rdata,"www.baidu.baifen.com");
    //     return 5;
    // }else{
    //     // 查询成功
    //     req_->rtype = req_->qtype;
    //     req_->rclass = 1;
    //     req_->ttl = 234;
    //     req_->rdata_len = 14;
    //     req_->rdata = (char *)malloc(sizeof(char) * 256);
    //     strcpy(req_->rdata,"110.114.117.6");
    //     return req_->qtype;
    // }
    return QUERY_FAIL;
}

// 根据上层发送数据更新数据库
int update_db(struct task * task_,int offset){
    
    struct dns_header *dnshdr = (struct dns_header *)task_->message;
    int ancount = ntohs(dnshdr->ancount);

    // 创建缓存区
    struct req req_ ;

    // 依次解析answer
    for(int i = 0;i < ancount;i++){
        offset += parse_to_data(task_->message + offset,&req_,task_->message);
        // 添加到缓存
        printf("======debug======\n");
        printf("req_ domain_name %s\n",req_.req_domain);
        printf("req_ rdata %s\n",req_.rdata);
        printf("req_ rdata_len %d\n",req_.rdata_len);
        printf("req_ ttl %d\n",req_.ttl);
        printf("req_ type %d\n",req_.rtype);

        if(trie_insert(trie_cache,req_.req_domain,req_.rtype,req_.rdata,req_.rdata_len,req_.ttl) == SUCCESS){
            LOG_INFO("update_db: insert %s\n",req_.req_domain);
        }else{
            LOG_ERROR("update_db: insert %s fail\n",req_.req_domain);
        }
    }
    return 0;
}


int throw_to_dns (struct task* task_){
    
    // 发送数据到dns服务器
    int state = talk_to_dns(task_->message,task_->m_len);
    
    // 依次打印message
    if(state == -1 || state == 0){
        // 超时或者查询失败
        LOG_ERROR("throw_to_dns: talk_to_dns fail\n");
        return -1;
    }else{
        task_->m_len = state;
        LOG_INFO("throw_to_dns: talk_to_dns success\n");
    }
    
    // 给客户端发送过去
    send_to_client(task_->message,task_->m_len,(struct sockaddr *)task_->addr,task_->sock_len);
    
    return 0;
}