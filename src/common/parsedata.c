#include "logger.h"
#include "parsedata.h"
#include "task.h"
#include <stdio.h>

int parse_to_domain(char * message,char * domain,int len){
    int i = 0, j = 0;
    while (message[i] != 0) {
        if(i > len){
            write_log(LOG_LEVEL_FATAL,"parse_to_domain error");
            exit(1);
        }
        int len = message[i];
        for (int k = 0; k < len; k++) {
            domain[j++] = message[++i];
        }
        domain[j++] = '.';
        i++;
    }
    domain[j - 1] = '\0'; // 去除最后一个'.'
    
    return i + 4;
}

int parse_to_domains(char * message,char * domains[]){
    struct dnsheader *dnshdr = (struct dnsheader *)message;
    int qdcount = ntohs(dnshdr->qdcount);

    domains = (char **)malloc(sizeof(char *) * qdcount);
    
    char * domain = NULL;
    
    
    // 从12字节后解析，跳过头部
    for(int i = 0,offset = 12;i < qdcount;i++){
        domain = (char *)malloc(sizeof(char) * 256);
        
        if(domain == NULL){
            write_log(LOG_LEVEL_FATAL,"malloc failed");
            exit(1);
        }

        offset += parse_to_domain(buf + offset, domain, 256);
        write_log(LOG_LEVEL_INFO,"query name: %s",domain);
        domains[i] = domain;
    }

    write_log(LOG_LEVEL_INFO,"query name: %s",dns_name);
    return qdcount;
}

void parse_add_dnsans(char *ip[],int len,char * message,int m_len){
    return;
}

void parse_add_dnsans(struct task * task_) {
    // 请求内存大小,按照最大的情况申请
    char *new_message = malloc(task_->m_len + task_->req_num*sizeof(dns_answer));
    
    if (!new_message) {
        write_log(LOG_LEVEL_FATAL,"malloc fail\n");
        return;
    }

    // 复制原有报文
    memcpy(new_message,task_->message,task_->m_len);

    // 修改相应头文件
    new_message[2] |= 0x80;  
    new_message[3] |= task_->req_num;

    // 追加指针
    char *answer_ptr = new_message + task_->m_len;
    // 定义缓存区域
    struct dns_answer* answer = (struct dns_answer*)malloc(sizeof(struct dns_answer));
    
    // 依次追加
    for (int i = 0; i < task_->req_num; i++) {
        // 创建answer报文
        
        parse_to_answer(task_->req[i])
        // 添加到报文尾部
        memcpy(answer_ptr, &answer, sizeof(answer));
        answer_ptr += sizeof(answer);

    }
    
    // Free the memory allocated for the new message
    free(new_message);
    new_message = NULL;
}

// 根据请求填充answer,返回域名填充偏移
int parse_to_answer(struct req req_,struct dns_answer * answer,char *message){
        // 填充answer 从后向前填充
        size_t len = strlen(message + req_->offset);

        memcpy(answer + 255 - len - 1, str, len + 1);

        switch (req_->type)
        {
        case 1:
            // IPV4
            answer->type = htons(req_->type);   // Type A (IPv4 address)    
            answer->rdlength = htons(4);  // Length of the RDATA field
            inet_pton(AF_INET,req_->ip,answer->rdata)
            break;
        case 28:
            // IPV6
            answer->type = htons(req_->type);   // Type A (IPv4 address)    
            answer->rdlength = htons(16);  // Length of the RDATA field
            inet_pton(AF_INET6,req_->ip,answer->rdata)
        default:
            // CNAME MX TXT
            break;
        }
        answer->class = htons(1);  // Class IN (Internet)
        answer->ttl = htonl(3600); // Time to live in seconds
        
        return 255 - len - 1;
}

int parse_to_answer(struct req* req_,char* answer){
        
        // req 是内含元数据

        int len = 2;
        
        // 列出各个字段的位置
        
        int16 * name = (int16 *)answer; // 使用指针实现

        int16 * type = (int16 *)(answer + len );
        int16 * class = (int16 *)(answer + len + 2);
        int32 * ttl = (int32 *)(answer + len + 4);
        int16 * rdlength = (int16 *)(answer + len + 8);
        
        int32 * rdata = (int32*) (answer + len + 10);
                
        // 写入域名
        *name = htons(0xc00c);
        *class = htons(1);  // Class IN (Internet)
        *ttl = htonl(3600); // Time to live in seconds

        // 写入类型
        switch(req_->type){
            case 1:
                // IPV4
                *type = htons(1);
                *rdlength = htons(4);
                *rdata = inet_addr(req_->ip);
                // // IPV6
                // *rdata = htonl();
                break;
            case 28:
                *type = htons(28);
                break;
            default:
                *type = htons(5);
                break;
        }

        // 返回answer的长度
        return len + 10 + 4;
}