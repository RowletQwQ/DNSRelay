#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"
#include "thpool.h"

// 任务池
struct list_ops_unit task_pool;
int task_free_flag = 0;

int main(){
    // 解析命令行参数 读文件
    
    // 创建任务池
    task_pool = linked_list_create();
    
    thread_pool pool = thpool_create(2);


    printf("task_pool create success\n");
    
    // 初始化socket IP可以指定
    socket_init();

    printf("socket_init() success\n");
    
    // 创建线程运行下面两个程序
    // pthread_t thread_listen,thread_manager;

    int ret;
    
    // void (*func1)(void *) = &socket_req_listen;

    // void (*func2)(void *) = &taskmanager;

    ret = thpool_add_work(pool, socket_req_listen, NULL);
    //ret = pthread_create(&thread_listen, NULL,func1, NULL);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    ret = thpool_add_work(pool, taskmanager, NULL);
    //ret = pthread_create(&thread_manager, NULL,func2, NULL);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    
    // pthread_join(thread_listen, NULL);
    // pthread_join(thread_manager, NULL);
    thpool_wait(pool);
    sleep(5);
    thpool_destroy(pool);
    return 0;
}