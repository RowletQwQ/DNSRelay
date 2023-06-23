//数据库交互访问层
#include "datatype.h"

#ifndef DAO_H
#define DAO_H

//常量
#define DAO_SUCCESS 0
#define DAO_FAILURE -1


//暴露增删改查的接口
//增
/**
 * @brief 插入一条记录
 * 
 * @param data_type 插入记录的类型
 * @param data 插入记录的数据,指向一个结构体
 * @param size 插入记录的数据大小
 * @return int 0表示成功,其他表示失败
 */
int insert_record(uint8 data_type,void* data,int size);

//查
/**
 * @brief 查询记录
 * 
 * @param data_type 查询记录的类型 
 * @param data 查询的条件,指向一个结构体
 * @param buf 查询结果的数据,指向一个结构体
 * @param buf_size 传入的缓冲区大小
 * @return int 0表示成功,其他表示失败
 */
int select_record(uint8 data_type,void* data,int size,void** buf,int buf_size);

//改
/**
 * @brief 更新记录
 * 
 * @param data_type 更新记录的类型
 * @param data 更新记录的数据,指向一个结构体
 * @param size 更新记录的数据大小
 * @return int 0表示成功,其他表示失败
 */
int update_record(uint8 data_type,void* data,int size);

//删
/**
 * @brief 删除记录
 * 
 * @param data_type 删除记录的类型
 * @param data 删除记录的数据,指向一个结构体
 * @param size 删除记录的数据大小
 * @return int 0表示成功,其他表示失败
 */
int delete_record(uint8 data_type,void* data,int size);
#endif