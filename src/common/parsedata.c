#include "parsedata.h"
#include "taskworker.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 根据系统引入不同的头文件
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

int parse_to_req(const char *buffer,struct req * req_) {
    
    req_->req_domain = (char *)malloc(256);
    char*dns_name = req_->req_domain;

    int i = 0, j = 0;
    while (buffer[i] != 0) {
        if(i > 256){
            exit(1);
        }
        int len = buffer[i];
        for (int k = 0; k < len; k++) {
            dns_name[j++] = buffer[++i];
        }
        dns_name[j++] = '.';
        i++;
    }
    dns_name[j - 1] = '\0'; // 去除最后一个'.'
    
    // 读取qtype 2字节
    req_->qtype = ntohs(*(uint16 *)(buffer + i + 1));
    req_->domain_len = i + 4;

    return i + 4;
}

int parse_to_answer(const struct req* req_,char* answer,const char *message){
        int len = 2;
        switch (req_->qtype)
        {
        case 1:
            len = 2;
        case 28:
            len = 2;
            int16 * name = (int16 *)answer; // 使用指针实现
            int16 pointer = 0xc000 | req_->domain_pointer;
            *name = htons(pointer); // 指针
            break;
        case 5:
            len = 2;
            break;
        default:
            // 不再是指针
            break;
        }
        // 写入其他指针
        int16 * type = (int16 *)(answer + len );

        int16 * class = (int16 *)(answer + len + 2);
        *class = htons(1);  // Class IN (Internet)

        int32 * ttl = (int32 *)(answer + len + 4);
        *ttl = htonl(3600); // Time to live in seconds

        int16 * rdlength = (int16 *)(answer + len + 8);
        
        char* rdata = (char*) (answer + len + 10);
        // 默认指针方法
    
        // 写入类型
        switch(req_->qtype){
            case 1:
                *type = htons(1);
                *rdlength = htons(4);
                // *rdata = inet_addr(req_->rdata);
                inet_pton(AF_INET, req_->rdata, rdata);
                break;
            case 28:
                *type = htons(28);
                *rdlength = htons(16);
                inet_pton(AF_INET6, req_->rdata, rdata);
                break;
            case 5:
                *type = htons(5);
                break;
            case 2:
                *type = htons(2);
                break;
            case 6:
                *type = htons(6);
                break;
            default:
                *type = htons(5);
                break;
        }
        return len + 10 + 4;
}

void parse_to_dnsres(struct task * task_) {
    // 请求内存大小,按照最大的情况申请

    // 用new_message指向task_的message
    char * new_message = task_->message;
    // 修改相应头文件
    new_message[2] |= 0x80;  
    new_message[3] |= task_->req_num;
    
    // 设置rcode为0 -- 假定都查询成功
    new_message[3] &= 0xf0;
    
    // 修改回答数量
    *((unsigned short *)(new_message + 6)) = htons(task_->req_num);

    // 追加指针
    char *answer_ptr = new_message + task_->m_len;
    
    // 定义缓存区域
    char* answer = (char*)malloc(sizeof(struct dns_answer));
    
    // 依次追加到报文上
    for (int i = 0; i < task_->req_num; i++) {
        // 初始化answer
        memset(answer,0,sizeof(struct dns_answer));
        
        // 解析req
        struct linked_list_node * req = linked_list_delete_head(task_->reqs);
        
        struct req * req_ = (struct req *)req->data;
        int answer_size =  parse_to_answer(req_,answer,task_->message);
        
        // 判断是否越界
        if(task_->m_len + answer_size > 512){
            // 判定该请求失败   
            printf("The answer is too long!\n") ;
            exit(1);
        }
        
        task_->m_len += answer_size;
        memcpy(answer_ptr,answer,answer_size);
        answer_ptr += answer_size;
        // 长度更新
    }
    // 将报文写入dns.txt
    FILE *fp = fopen("dns.txt","wb");
    fwrite(task_->message,1,task_->m_len,fp);
    fclose(fp);
}

int parse_to_reqs(struct task * task_){
    
    // 解析报文头
    struct dns_header *dnshdr = (struct dns_header *)task_->message;
    // 解析请求数目
    task_->req_num = ntohs(dnshdr->qdcount);
    // 创建链表
    task_->reqs = linked_list_create();
    // 解析请求
    char * buf = task_->message;
    
    for(int i = 0,offset = 12;i < task_->req_num;i++){
        printf("Query No.%d\n",i);
        struct req req_;
        
        req_.domain_pointer = (int16) offset;    
        
        // 计算偏移
        offset += parse_to_req(buf + offset,&req_);
        printf("%s\n",req_.req_domain);
        // 添加到链表中
        if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
            return 0;
            printf("add failed\n");
        }
        
    }

    return 1;
}