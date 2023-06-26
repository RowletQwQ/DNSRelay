#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "thpool.h"
#include "logger.h"
#include "setting.h"
#include "dao.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
// 任务池
thread_pool tasker;
struct list_ops_unit task_pool;
struct trie_node * trie_cache;
int task_free_flag = 0;



int main(int argc, char *argv[]){
    // 解析命令行参数 读文件
    say_hello();

    parse_args(argc,argv);
    
    // 初始化用户文件
    init_read_file(get_user_file());
    tasker = thpool_create(10);
    // LOG
    init_log(get_log_file(),get_debug_level(),0,tasker);

    // 初始化DAO层
    init_dao();

    // 初始化socket IP可以指定
    socket_init();
    
    
    // 监听接口
    socket_req_listen();
    
    thpool_wait(tasker);
    Sleep(5000);
    thpool_destroy(tasker);
    return 0;
}