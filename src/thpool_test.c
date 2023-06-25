#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include "logger.h"
#include "thpool.h"

void print_int(void* arg) {
    intptr_t n = (intptr_t)arg;
    printf("Thread #%u working on task,number %d\n", (int)pthread_self(),(int)n);
}

int main(int argc, char* argv[]) {
    
    // 0.初始化日志打印
    if(argc < 3) {
        printf("usage: %s <log_file_name> <DEBUG_LEVEL>\n", argv[0]);
        printf("DEBUG_LEVEL: 0-DEBUG, 1-INFO, 2-WARN, 3-ERROR\n");
        return 0;
    }
    thread_pool log_pool = thpool_create(1);
    init_log(argv[1], atoi(argv[2]),0,log_pool);
    // 1.创建线程池
    struct thread_pool_t* pool = thpool_create(8);
    
    
    // 2.创建任务
    for(int i = 0; i < 10000; i++) {
        //printf("add work %d\n", i);
        thpool_add_work(pool, print_int, (void*)(uintptr_t)i);
    }
    //printf("add work done\n");
    thpool_wait(pool);
    // 3.销毁线程池
    printf("destroy thread pool\n");
    thpool_destroy(pool);
    printf("main thread exit\n");
    thpool_wait(log_pool);
    thpool_destroy(log_pool);
    return 0;
}
