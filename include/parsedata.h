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
 * @brief 解析DNS报文中的查询名到域名组
 * 
 * @param message DNS报文
 * @param domains 域名组
 * @return int 域名组的个数
 */
int parse_to_domains(char * message,char * domains[])

/**
 * @brief 依次解析DNS报文的查询名到域名
 * 
 * @param message 部分DNS报文
 * @param domains 域名
 * @return int 域名的偏移量
 */
int parse_to_domain(char * buffer,char * domain,int len);

/**
 * @brief 增添ip数据到DNS应答报文中
 * 
 * @param ip ip地址
 * @param message DNS应答报文
 */
void parse_add_dnsans(char *ip[],int len,char * message,int m_len);

/**
 * @brief 把req请求回答添加到answer中
 * 
 * @param req 请求和响应内容
 * @param message DNS应答报文缓存区
 */
int parse_to_answer(struct req* req_,char* answer);


#endif