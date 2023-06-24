#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "logger.h"
#include "thpool.h"

void print_int(int num) {
    printf("%d\n", num);
}

int main() {
    // 1.创建线程池
    struct thread_pool *pool = thpool_create(40);

    // 2.创建任务
    for(int i = 0; i < 100; i++) {
        struct task *task = task_create(print_int, i);
        thpool_add_task(pool, task);
    }
    thpool_wait(pool);
    // 3.销毁线程池
    thpool_destroy(pool);
    return 0;
}
