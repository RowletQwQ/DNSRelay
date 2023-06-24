#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "socket.h"
#include "taskworker.h"
#include "linked_list.h"

// 任务池
struct list_ops_unit task_pool;
int task_free_flag = 0;

int main(char argc, char *argv[]){
    // 解析命令行参数 读文件
    
    // 创建任务池
    task_pool = linked_list_create();

    printf("task_pool create success\n");
    
    // 初始化socket IP可以指定
    socket_init();

    printf("socket_init() success\n");
    
    // 创建线程运行下面两个程序
    pthread_t thread_listen,thread_manager;

    int ret;

    ret = pthread_create(&thread_listen, NULL, (void *)socket_req_listen, NULL);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&thread_manager, NULL, (void *)taskmanager, NULL);
    if (ret != 0) {
        printf("Error creating thread: %d\n", ret);
        exit(EXIT_FAILURE);
    }
    
    pthread_join(thread_listen, NULL);
    pthread_join(thread_manager, NULL);

    return 0;
}