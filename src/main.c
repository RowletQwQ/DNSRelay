#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "thpool.h"

#include "trie.h"
#include "logger.h"
// 任务池
struct list_ops_unit task_pool;
// 字典树缓存
struct trie_node * trie_cache;
int task_free_flag = 0;


int main(){
    // 初始化日志
    thread_pool log_worker = thpool_create(5);
    init_log("test.log",LOG_LEVEL_DEBUG,1,log_worker);
    // 解析命令行参数 读文件

    // 初始化字典树
    trie_cache = trie_create();
    LOG_INFO("tire_create success\n");
    
    // 创建任务池
    task_pool = linked_list_create();
    LOG_INFO("task_pool create success\n");
    
    socket_init();
    LOG_INFO("socket_init success\n");
    // // 创建先乘除
    // thread_pool pool = thpool_create(2);
    // LOG_INFO("thread_pool create success\n");
    
    // // 创建线程运行下面两个程序
    // int ret;
    
    // ret = thpool_add_work(pool, socket_req_listen, NULL);
    // if (ret != 0) {
    //     printf("Error creating thread: %d\n", ret);
    //     exit(EXIT_FAILURE);
    // }
    // LOG_INFO("socket_req_listen success\n");


    // ret = thpool_add_work(pool, taskmanager, NULL);
    // if (ret != 0) {
    //     printf("Error creating thread: %d\n", ret);
    //     exit(EXIT_FAILURE);
    // }
    // // 等待线程池结束
    // thpool_wait(pool);
    // thpool_destroy(pool);


    pthread_t thread_listen,thread_manager;

    int ret;

    ret = pthread_create(&thread_listen, NULL, (void *)socket_req_listen, NULL);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    // ret = pthread_create(&thread_manager, NULL, (void *)taskmanager, NULL);
    // if (ret != 0) {
    //     printf("Error creating thread: %d\n", ret);
    //     exit(EXIT_FAILURE);
    // }

    pthread_join(thread_listen, NULL);
    // pthread_join(thread_manager, NULL);
    
    // 释放字典树
    free_trie_node(trie_cache);
    return 0;
}