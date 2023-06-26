#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "trie.h"
#include "thpool.h"
// 任务池
thread_pool tasker;
struct list_ops_unit task_pool;
struct trie_node * trie_cache;
int task_free_flag = 0;



int main(char argc, char *argv[]){
    // 解析命令行参数 读文件
    
    // 创建任务池
    task_pool = linked_list_create();

    printf("task_pool create success\n");
    
    trie_cache = trie_create();
    printf("trie_cache create success\n");

    // 初始化socket IP可以指定
    socket_init();

    printf("socket_init() success\n");
    
    // 创建线程运行下面两个程序
    pthread_t thread_listen;
    // pthread_t thread_manager;

    // 创建线程池
    tasker = thpool_create(10);
    printf("tasker create success\n");

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
    thpool_wait(pool);
    sleep(5);
    thpool_destroy(pool);
    return 0;
}