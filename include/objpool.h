#include "datatype.h"
//对象池
#ifndef _OBJPOOL_H_
#define _OBJPOOL_H_

//相关常量
#define OBJ_POOL_SUCCESS 0
#define OBJ_POOL_FAILURE -1

//对象池的结构体,在这里定义,在objpool.c中实现
typedef struct obj_pool_t* obj_pool;
//还要加入对线程池的支持
typedef struct thread_pool_t* thread_pool;

//对象池的初始化
/**
 * @brief 对象池的初始化
 * 
 * @param size 对象池的大小
 * @param obj_size 对象的大小
 * @param pool 线程池指针,用于对象池的线程安全
 * @return obj_pool 成功返回对象池指针,失败返回NULL
 */
obj_pool obj_pool_init(int size, int obj_size, thread_pool pool);

//对象池的销毁
/**
 * @brief 对象池的销毁
 * 
 * @param pool 对象池指针
 * @return int 成功返回OBJ_POOL_SUCCESS,失败返回OBJ_POOL_FAILURE
 */
int obj_pool_destroy(obj_pool pool);

//从对象池中申请一个对象
/**
 * @brief 对象池的申请
 * 
 * @param pool 对象池指针
 * @return void* 成功返回对象指针,失败返回NULL
 */
void* obj_pool_alloc(obj_pool pool);

//将对象归还给对象池
/**
 * @brief 对象池的归还
 * 
 * @param pool 对象池指针
 * @param obj 对象指针
 * @return int 成功返回OBJ_POOL_SUCCESS,失败返回OBJ_POOL_FAILURE
 */
int obj_pool_free(obj_pool pool, void* obj);

//销毁对象池
/**
 * @brief 销毁对象池
 * 
 * @param pool 对象池指针
 * @return int 成功返回OBJ_POOL_SUCCESS,失败返回OBJ_POOL_FAILURE
 */
int obj_pool_destroy(obj_pool pool);


#endif