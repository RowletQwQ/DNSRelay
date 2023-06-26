#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "trie.h"
#include "thpool.h"
#include "logger.h"
#include "setting.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
// 任务池
//thread_pool tasker;
struct list_ops_unit task_pool;
struct trie_node * trie_cache;
int task_free_flag = 0;



int main(char argc, char *argv[]){
    // 解析命令行参数 读文件
    say_hello();
    parse_args(argc,argv);
    trie_cache = trie_create();
    printf("trie_cache create success\n");
    
    // 初始化socket IP可以指定
    socket_init();

    printf("socket_init() success\n");
    
    // 创建线程运行下面两个程序
    //pthread_t thread_listen;
    
    // pthread_t thread_manager;
    init_log(get_log_file(),get_debug_level(),0,NULL);
    
    // 创建线程池
    // tasker = thpool_create(4);
    // printf("tasker create success\n");
    
    socket_req_listen();
    
    // thpool_wait(tasker);
    // Sleep(5000);
    // thpool_destroy(tasker);
    
    return 0;
}