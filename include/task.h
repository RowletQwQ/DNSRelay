// 任务相关
#include "datatype.h"
#include "linked_list.h"
struct task{
    int taskid; //指向任务的发送窗口id
    char* message;
    int m_len;
    int req_num;
    struct list_ops_unit reqs; //链表指针
};
struct req{
    char* req_domain; //请求域名
    int8 domain_len; //最大255
    int16 offset; //域名原始偏移数
    int8 qtype; //请求类型  响应部分
    int8 rtype; //查询到的ip类型
    char * rdata; //ip地址
};
