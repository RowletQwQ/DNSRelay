#include "datatype.h"
// 解析DNS报文相关
#ifndef _PARSEDATA_H_
#define _PARSEDATA_H_
// 宏定义
#define DNS_OK 0 //无错误
#define DNS_FORMAT 1 //格式错误
#define DNS_SERVER 2 //服务器错误
#define DNS_NOTEXIST 3 //域名不存在
#define DNS_QUERY_TYPE_ERROR 4 //查询类型错误
#define DNS_DEQUERY 5 //拒绝查询

struct task;
struct req;

// 协议结构

//question 查询部分 长度 不定
struct dns_question {
    char qname[255];     // 查询域名
    unsigned short qtype; // 查询类型 2
    unsigned short qclass; // 查询类 2
};

// DNS 报文头结构体 长度12
struct dns_header {
    unsigned short id;          // 报文 ID
    unsigned char qr:1,         // 查询/响应标志位
                  opcode:4,     // 操作码
                  aa:1,         // 授权回答标志位
                  tc:1,         // 截断标志位
                  rd:1,         // 递归查询标志位
                  ra:1,         // 递归可用标志位
                  z:3,          // 保留位
                  rcode:4;      // 响应码
    unsigned short qdcount;     // 问题部分的条目数
    unsigned short ancount;     // 回答部分的条目数
    unsigned short nscount;     // 授权部分的条目数
    unsigned short arcount;     // 附加部分的条目数
};

// DNS 回答记录结构体 最后的部分
struct dns_answer {
    char name[255];        // 域名
    unsigned short type;   // 记录类型
    unsigned short class;  // 记录类
    unsigned int ttl;      // 生存时间
    unsigned short rdlength;  // 数据长度
    unsigned int rdata;    // 数据
};
/**
 * @brief 将任务数据转换为DNS应答报文
 * @param task_ 处理好的任务
 */
void parse_to_dnsres(struct task * task_);

/**
 * @brief 把req转换成answer部分报文
 * 
 * @param req_ 单个查询任务请求和响应内容
 * @param answer DNS应答报文缓存区
 * @param message 报文数据
 * @return answer部分长度
 */
int parse_to_answer(const struct req* req_,char* answer);

/**
 * @brief 读取报文中的查询部分，填充到req_中
 * 
 * @param req_ 单个查询任务请求和响应内容
 * @param buffer 部分报文内容，待解析区
 * @return int 该查询部分长度
 */
int parse_to_req(const char *buffer,struct req * req_,const char* message);

int parse_to_reqs(struct task * task_);

/**
 * @brief 把报文转换成数据条目，更新到缓存和数据库中,并且对于CNAME的rdata中可能存在使用指针的情况，需要解析出来处理
 * 
 * @param message 报文数据 
 * @return int 转换成功的数据量
 */
int parse_to_data(const char *answer,struct req * req_,const char * message);

/**
 * @brief 把网络字符串域名转换成常用字符串域名
 * 
 * @param buf 部分报文数据 
 * @param str 常用字符串域名
 * @param message 报文数据
 * @param str_len 转换数据长度,
 * @return int 转换数据偏移量
 */
int parse_to_string(const char * buf,char * str,int16 *str_len,const char * message);


/**
 * @brief 把字符串转换成网络字符串
 * 
 * @param astr 部分报文数据 
 * @param a_len 字符串长度
 * @param nstr 网络字符串
 * @return int 网络字符串长度
 */
int parse_to_netstr(char * astr,char * nstr);

/**
 * @brief 规格化rdata
 * @param req_ 单个查询任务请求和响应内容
 * @return int 
 */
int parse_to_rdata(struct req* req_);
#endif