#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "taskworker.h"
#include "socket.h"
#include "parsedata.h"
#include "dao.h"
#include "logger.h"
extern struct list_ops_unit task_pool;

extern struct trie_node * trie_cache;

void taskworker(struct task * task_){
    
    task_ = (struct task *)task_;
    
    // 1.链式解析报文和查询报文
    int query_state = link_query_reqs(task_);

    if(query_state == QUERY_FAIL){
        // 快速响应
        printf("query fail\n");

        // 只更新message
        int offset = task_->m_len;
        int res = talk_to_dns(task_->message,task_->m_len,task_->addr,task_->sock_len);
        
        if(res == -1){
            printf("throw_to_dns failed!\n");
            // 销毁内存
        }else{
            update_db(task_,offset); // 更新数据库
        }

    }else{
        printf("==============local query success=========\n");
        // 查询成功
        parse_to_dnsres(task_);
        // 更新响应包
        printf("\nsento_client\n");
        // 发送响应包
        printf("%d\n",send_to_client(task_->message,task_->m_len,&task_->addr,task_->sock_len));
    };

    // 释放资源
    task_free(task_);
    return;
}

// 创建完整链式请求,创建req的地方
int link_query_reqs(struct task * task_) {
    // 解析报文头
    struct dns_header *dnshdr = (struct dns_header *)task_->message;
    // 解析请求数目
    task_->req_num = ntohs(dnshdr->qdcount);
    
    // 解析请求
    char * buf = task_->message;

    for(int i = 0,offset = 12;i < ntohs(dnshdr->qdcount);i++){    
        struct req temp_req;//局部
        
        // 后面释放
        temp_req.req_domain = (char *)malloc(DATA_MAX_BUF);
        
        // 初始化
        struct req req_ ;
        req_.req_domain = (char *)malloc(sizeof(char) * DATA_MAX_BUF);
        req_.rdata = (char *)malloc(sizeof(char) * DATA_MAX_BUF);
        req_.domain_len = 0;
        req_.rdata_len = 0;
        req_.domain_pointer = (int16) offset; //正常请求 初始化偏移值
        
        // 偏移增加 指向message中的消息 填充req_
        int res_toreq = parse_to_req(buf + offset,&req_,(const char *)task_->message);
        
        if(res_toreq == -1){
            printf("parse_to_req failed!\n");
            return -1;
        }

        printf("================req==============\n");
        printf("req_domain:%s\n",req_.req_domain);
        printf("req_domain_len:%d\n",req_.domain_len);
        printf("qtype %d\n",req_.qtype);

        // 更新偏移量
        offset += res_toreq;
        
        // 查询该记录 , 填充 , 记录resp的原本名字作为CNAME域名
        int resp_type = query_req(&req_); 

        if(resp_type == -1){
            // 查询失败 直接返回
            return -1;
        }

        // 记录域名 可能作为下一个CNAME查询
        memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
        temp_req.domain_len = req_.rdata_len;
        
        // 格式化该请求
        int res_tordata = parse_to_rdata(&req_);
        
        if(res_tordata == -1){
            printf("parse_to_rdata failed!\n");
            return -1;
        }
        
        // 添加到链表中
        if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
            printf("Insert req failed!\n");
            return -1;
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
                
                // 域名设置
                req_.req_domain = (char *)malloc(sizeof(char)*DATA_MAX_BUF);
                req_.rdata = (char *)malloc(sizeof(char)*DATA_MAX_BUF);

                memcpy(req_.req_domain,temp_req.req_domain,temp_req.domain_len);
                req_.domain_len = temp_req.domain_len;
                
                // 指向前一个 的负数
                req_.domain_pointer = -req_.rdata_len;

                // qtype 和 qclass不变

                // 查询这个新的请求
                resp_type = query_req(&req_);
                if(resp_type == -1){
                    return -1;
                }
                // 打印req
                
                // 再次记录下
                memcpy(temp_req.req_domain,req_.rdata,req_.rdata_len);
                temp_req.domain_len = req_.rdata_len;
                
                //格式化
                int to_rdata_state = parse_to_rdata(&req_);
                if(to_rdata_state == -1){
                    printf("parse_to_rdata failed!\n");
                    return -1;
                }

                // 添加到链表中
                if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
                    printf("Insert req failed!\n");
                    return -1;
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
    
    // 1.释放message报文
    if(task_->message != NULL){
        free(task_->message);
    }

    // 2.释放req请求
    struct linked_list_node* req_ = linked_list_delete_head(task_->reqs);
    while(req_!=NULL){
        // 释放req
        struct req * data = (struct req *)req_->data;
        free(data->req_domain);
        free(data->rdata);
        free(req_);
        req_ = linked_list_delete_head(task_->reqs);
    }

    // 3.释放task
    free(task_);
    return;
}

// 返回响应类型
int query_req(struct req * req_){
    
    DNSRecord record;
    printf("=====QUERY: req_domain:%s=======\n",req_->req_domain);

    int res = query_record(req_->req_domain,req_->qtype,&record);
    
    if(res != DAO_FAILURE){
        printf("==============query in local=============\n");
        // 打印消息
        printf("rdata:%s\n",record.record);
        printf("rdata_len:%d\n",record.record_len);
    }else{
        // 查CNAME
        res = query_record(req_->req_domain,CNAME,&record);
        if(res != DAO_FAILURE){
            printf("==============query in local=============\n");
            printf("rdata:%s\n",record.record);
            printf("rdata_len:%d\n",record.record_len);
        }
    }
    

    if(res == DAO_FAILURE){
        // 查询失败
        return QUERY_FAIL;
    }else {
        if(record.record_type == req_->qtype || record.record_type == CNAME){
            req_->rtype = record.record_type;
            req_->rclass = 1;
            req_->ttl = record.expire_time - time(NULL);
            req_->rdata_len = record.record_len;
            memcpy(req_->rdata,record.record,record.record_len);
            return req_->rtype;
        }else{
            return QUERY_FAIL;
        }
    }
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
        offset += parse_to_data(task_->message + offset,&req_,task_->message);
        if(offset == -1){
            printf("parse_to_data failed!\n");
            return -1;
        }

        // 封口
        req_.req_domain[(int)req_.domain_len] = '\0';
        req_.rdata[req_.rdata_len] = '\0';
        
        // 添加到缓存
        printf("======+++++++++dns data++++++++======\n");
        printf("req_ domain_name %s\n",req_.req_domain);
        
        printf("req_ rdata_len %d\n",req_.rdata_len);
        printf("req_ ttl %d\n",req_.ttl);
        printf("req_ type %d\n",req_.rtype);
        if(req_.domain_len > 0 && req_.domain_len < 256 && req_.rdata_len > 0 && req_.rdata_len < 256 && req_.rtype > 0){
            // 判定数据插入条件
            if(add_record(req_.req_domain,req_.rtype,req_.rdata,req_.rdata_len,req_.ttl) == DAO_FAILURE){
                LOG_ERROR("update_db : add_record failed!");
            }else{
                printf("update_db : add_record %s success!\n",req_.req_domain);
            }
        }
    }
}
