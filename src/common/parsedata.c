#include "parsedata.h"
#include "taskworker.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 根据系统引入不同的头文件
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

int parse_to_req(const char *buffer,struct req * req_,const char* message) {

    int16 str_len = 0;
    int i = parse_to_string(buffer,req_->req_domain,&str_len,message);
    
    if(str_len > 255){
        printf("domain too long\n");
        return -1;
    }

    req_->domain_len = (unsigned char) str_len;
    
    // 读取qtype 2字节
    req_->qtype = ntohs(*(uint16 *)(buffer + i ));
    
    // 读取qclass 2字节
    req_->qclass = ntohs(*(uint16 *)(buffer + i + 2));

    return i + 3;
}

int parse_to_answer(const struct req* req_,char* answer){        
        
        int len = 2;
        // 默认三种是指针方法
        int16 * name = (int16 *)answer; // 使用指针实现
        int16 pointer = 0x0000c000 | req_->domain_pointer;
        
        *name = htons(pointer); // 指针
        
        printf("pointer %02x\n",*name);

        
        // 写入其他指针
        int16 * type = (int16 *)(answer + len );

        int16 * class = (int16 *)(answer + len + 2);
        *class = htons(req_->rclass);  

        int32 * ttl = (int32 *)(answer + len + 4);
        *ttl = htonl(req_->ttl); 

        int16 * rdlength = (int16 *)(answer + len + 8);
        
        // 可以是IP或者字符串
        char* rdata = (char*) (answer + len + 10);
        if(req_->rdata_len > 255){
            return -1;
        }
        // 写入类型
        switch(req_->rtype){
            case 1:
                *type = htons(1);
                *rdlength = htons(4);
                memcpy(rdata,req_->rdata,4);
                break;
            case 28:
                *type = htons(28);
                *rdlength = htons(16);
                memcpy(rdata,req_->rdata,16);
                break;
            case 5:
                *type = htons(5);
                *rdlength = htons(req_->rdata_len);
                // 最大256
                memcpy(rdata,req_->rdata,req_->rdata_len);
                break;
            case 2:
                *type = htons(2);
                memcpy(rdata,req_->rdata,req_->rdata_len);
                break;
            case 6:
                *type = htons(6);
                memcpy(rdata,req_->rdata,req_->rdata_len);
                break;
            default:
                *type = htons(5);
                memcpy(rdata,req_->rdata,req_->rdata_len);
                break;
        }
        
        return len + 10 + req_->rdata_len;
}

void parse_to_dnsres(struct task * task_) {
    char * new_message = task_->message;
    
    // 修改相应头
    new_message[2] |= 0x80;  // 响应标志
    
    // 设置rcode为0 -- 假定都查询成功
    new_message[3] &= 0xf0;
    
    // 修改回答数量
    *((unsigned short *)(new_message + 6)) = htons(task_->req_num);

    // 追加指针
    char *answer_ptr = new_message + task_->m_len;
    
    // 定义缓存区域 下面释放
    char* answer = (char*)malloc(sizeof(struct dns_answer));

    int num = task_->req_num;
    for (int i = 0; i < num; i++) {
        printf("==================>i: %d\n",i);
        // 初始化answer
        memset(answer,0,sizeof(struct dns_answer));
        
        // 解析req 从链表中取出，链表中已经删除，内存需要手动释放
        struct linked_list_node * req = linked_list_delete_head(task_->reqs);
        
        if(req == NULL){
            printf("req is null!\n");
            return;
        }
        
        assert(req->data_len == sizeof(struct req));
        struct req * req_ = (struct req *)req->data;

        // 指向指针
        if(req_->domain_pointer < 0){
            req_->domain_pointer = task_->m_len + req_->domain_pointer;
        }

        int answer_size =  parse_to_answer(req_,answer);
        
        // 判断是否越界
        if(task_->m_len + answer_size > 512){
            // 判定该请求失败   
            printf("The answer is too long!\n") ;
            return;
        }

        printf("answer_size: %d\n",answer_size);

        // 长度更新
        task_->m_len += answer_size;
        memcpy(answer_ptr,answer,answer_size);
        answer_ptr += answer_size;
        
        // 内存释放
        free(req_->req_domain);
        free(req_->rdata);
    }
    free(answer);
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
        offset += parse_to_req(buf + offset,&req_,task_->message);
        printf("%s\n",req_.req_domain);
        // 添加到链表中
        if(linked_list_insert_tail(task_->reqs, (int8 *)&req_,sizeof(struct req)) == -1){
            return 0;
            printf("add failed\n");
        }
        
    }

    return 1;
}

// 把buf解析成字符串域名类型
int parse_to_string(const char * buf,char * str,int16* str_len, const char * message){

    // 根据buf填充str 但是存在各种指针，需要特征判断，当下是按照指针解析还是数字解析
    int i = 0;
    int16 j = 0;
    while (buf[i] != 0) {

        if(i > 256){
            printf("parse_to_req: domain name too long!\n");
            return -1;
        }
        // 判断是指针还是长度
        if(buf[i] & 0xc0){
            // 指针
            int16 * pointer = (int16 *) (buf + i);   
            int16 offset = ntohs(*pointer);
            offset &= 0x00003fff;
            
            i += 2; // 加二
            
            // 指向最后的域名,可能存在多次跳转
            while(offset & 0x0000c000){
                if(offset & 0x00003fff > 512){
                    printf("parse_to_req: domain name too long!\n");
                    return -1;
                }
                pointer = (int16 *)(message + (offset & 0x00003fff));
                offset = ntohs(*pointer);
            }

            // 跳过计数
            offset++;
            
            // 不是指针了 直接把剩下的拷贝过来
            while(message[offset] != 0){
                str[j++] = message[offset++];
                if(j > 256){
                printf("parse_to_req: domain name too long!\n");
                return -1;
            }
            }
            // 结束了
            str[j++] = '.';
            break;
        }else{
            // 长度 是数字
            int len = buf[i];
            for(int k = 0;k < len;k++){
                str[j++] = buf[++i];
            }
            str[j++] = '.';
            i++;
        }
    }
    *str_len = j ;

    str[j-1] = '\0'; // 去除最后一个'.'
    printf("DEBUG : %d\n",j);
    printf("str: %s\n",str);
    return i + 1;
}

int parse_to_data(const char *answer,struct req * req_,const char * message){

    // 解析name
    int16 * name = (int16 *)answer;
    
    int16 pointer = ntohs(*name);
    // 判断是否是指针
    
    int len = 0;
    if(pointer & 0xc000){
        req_->domain_pointer = pointer & 0x00003fff;
        len = 2;
        req_->req_domain = (char*)malloc(sizeof(char) * DATA_MAX_BUF);
        int16 str_len = 0;
        if(parse_to_string(message + req_->domain_pointer,req_->req_domain,&str_len,message) == -1){
            printf("parse_to_data: parse_to_string failed! \n");
            return -1;
        }
        req_->domain_len = (int8)str_len;
    }else{
        // 不是指针
        req_->domain_pointer = 0;
        req_->req_domain = (char*)malloc(sizeof(char) * DATA_MAX_BUF);
        int16 str_len = 0;
        len = parse_to_string(answer,req_->req_domain,&str_len,message);
        if(len == -1){
            return -1;
        }
        req_->domain_len = (int8)str_len;
    }
    // 解析type
    
    int16 * type = (int16 *)(answer + len);
    req_->rtype = ntohs(*type);
    
    // 解析class
    int16 * class = (int16 *)(answer + len + 2);
    req_->rclass = ntohs(*class);

    // 解析ttl
    int32 * ttl = (int32 *)(answer + len + 4);
    req_->ttl = ntohl(*ttl);
    
    // 解析rdlength
    int16 * rdlength = (int16 *)(answer + len + 8);
    int16 rdl = ntohs(*rdlength);
    
    
    // 解析rdata
    char * rdata = (char *)(answer + len + 10);
    req_->rdata = rdata;
    // 根据rdata的类型解析 
    req_->rdata = (char *)malloc(sizeof(char) * 256);
    if(req_->rtype == 1){
        req_->rdata_len = 4;
        memcpy(req_->rdata,rdata,4);
    }else if(req_->rtype == 5){
        // CNAME
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 15){
        // MX
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 2){
        // NS
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 6){
        // SOA
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 12){
        // PTR
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 16){
        // TXT
        if(parse_to_string(rdata,req_->rdata,&req_->rdata_len,message) == -1){
            return -1;
        }
        req_->rdata_len = req_->rdata_len;
    }else if(req_->rtype == 28){
        // AAAA
        req_->rdata_len = 16;
        memcpy(req_->rdata,rdata,16);
    }
    // 返回这个answer的长度
    return len + 10 + rdl;
}

int parse_to_netstr(char * astr,char * nstr){
    
    char * old_nstr = nstr;
    int len = 0;
    // 不等于零，并且大小不超过a_len
    while (*astr != '\0') {
        
        // 找到下一个标签
        char *next_label = strchr(astr, '.');
        
        if (next_label == NULL) {
            next_label = astr + strlen(astr);
        }else if(next_label - astr > 63){
            printf("parse_to_netstr: label len > 63!\n");
            return -1;
        }

        // 计算标签的长度，并将其添加到DNS报文中
        int label_len = next_label - astr;
        *nstr++ = label_len;
        len++;

        // 将标签的字符添加到DNS报文中
        for (int i = 0; i < label_len; i++) {
            *nstr++ = *astr++;
            len++;
            // 长度不能超过255
            if(len > 255){
                printf("parse_to_netstr: len > 255!\n");
                return -1;
            }
        }

        // 如果还有更多的标签，则在标签之间添加一个“.”（长度为0）
        if (*next_label == '.') {
            astr++;
        }
    }
    
    // 封最后的0
    *nstr++ = '\0';
    return nstr - old_nstr;
}

int parse_to_rdata(struct req* req_){
    char *netstr = NULL;
    switch(req_->rtype){
            case 1:
                // IPV4
                // inet_pton(AF_INET, req_->rdata, req_->rdata);
                
                // req_->rdata_len = 4;
                break;
            case 28:
                // inet_pton(AF_INET6, req_->rdata, req_->rdata);
                // req_->rdata_len = 16;
                break;
            case 5:
                netstr = (char *)malloc(sizeof(char) * 256);
                int netstr_len = parse_to_netstr(req_->rdata,netstr);

                if(netstr_len == -1){
                    printf("parse_to_rdata: parse_to_netstr failed!\n");
                    free(netstr);
                    return -1;
                }

                memcpy(req_->rdata,netstr,netstr_len);
                // 释放内存
                free(netstr);
                req_->rdata_len = netstr_len;
                break;
            case 2:
                break;
            case 6:
                break;
            default:
                break;
        }
    return 1;
}