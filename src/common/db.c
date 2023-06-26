#include "db.h"
#include "sqlite3.h"
#include "userfile.h"
#include "logger.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 把用户自定义的域名信息插入到数据库中
static int32 insert_user_setting_domin_info();

// 把用户自定义的域名信息插入到数据库中
static int32 insert_user_setting_domin_info() {
    // 创建一个domin_table_data结构体指针得到数据
    struct domin_table_data *domin_table_data_ptr = NULL;
    int len = read_data(&domin_table_data_ptr);
    // 先删除数据库中expire_time为-1的值, 也就是用户自定义的域名信息
    char *delete_sql = "DELETE FROM domin_table WHERE expire_time = -1;";
    char *err_msg = NULL;
    sqlite3 *db = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        LOG_ERROR("open db failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_exec(db, delete_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        LOG_ERROR("delete failed");
        sqlite3_close(db);
        return FAIL;
    }
    // 插入数据
    char *insert_sql = "INSERT INTO domin_table(domin_name, record_type, record, record_len, expire_time) VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("prepare failed");
        sqlite3_close(db);
        return FAIL;
    }
    // 绑定数据
    for (int i = 0; i < len; i++) {
        ret = sqlite3_bind_text(stmt, 1, domin_table_data_ptr[i].domin_name, -1, NULL);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind domin_name failed");
            sqlite3_close(db);
            return FAIL;
        }
        ret = sqlite3_bind_int(stmt, 2, domin_table_data_ptr[i].record_type);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind record_type failed");
            sqlite3_close(db);
            return FAIL;
        }
        ret = sqlite3_bind_blob(stmt, 3, domin_table_data_ptr[i].record, domin_table_data_ptr[i].record_len, NULL);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind record failed");
            sqlite3_close(db);
            return FAIL;
        }
        ret = sqlite3_bind_int(stmt, 4, domin_table_data_ptr[i].record_len);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind record_len failed");
            sqlite3_close(db);
            return FAIL;
        }
        ret = sqlite3_bind_int64(stmt, 5, domin_table_data_ptr[i].expire_time);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind expire_time failed");
            sqlite3_close(db);
            return FAIL;
        }

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            LOG_ERROR("insert failed");
            sqlite3_close(db);
            return FAIL;
        }

        // 重置
        ret = sqlite3_reset(stmt);
        if (ret != SQLITE_OK) {
            LOG_ERROR("reset failed");
            sqlite3_close(db);
            return FAIL;
        }
    }

    // 释放资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return SUCCESS;
}

// 1.初始化数据库, 建表
int32 init_db() {
    // 先打开数据库, 如果数据库不存在, 则创建数据库
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        LOG_ERROR("open db failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 根据.h文件中定义的表结构, 创建表, domin_name不再为主键, 而是一个索引, 但不是唯一索引
    char *create_table_sql = "CREATE TABLE IF NOT EXISTS domin_table("
                             "domin_name TEXT NOT NULL,"
                             "record_type INTEGER NOT NULL,"
                             "record BLOB NOT NULL,"
                             "record_len INTEGER NOT NULL,"
                             "expire_time INTEGER NOT NULL,"
                             "PRIMARY KEY(domin_name, record_type, record, record_len, expire_time));";
    
    ret = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        LOG_ERROR("create table failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    // 把用户自定义的域名信息插入到数据库中
    if (insert_user_setting_domin_info() == SUCCESS) {
        LOG_ERROR("insert user setting domin info success");
    } else {
        LOG_ERROR("insert user setting domin info failed");
    }

    return SUCCESS;
}

// 2.根据域名和记录类型进行查询, 结果为NULL表示查询失败
struct record_dto *query_by_domin_name(const char *domin_name, uint16 record_type) {
    // 打开数据库
    sqlite3 *db = NULL;
   // char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        LOG_ERROR("open db failed");
        sqlite3_close(db);
        return NULL;
    }

    // 查询
    char *query_sql = "SELECT record, record_len, expire_time FROM domin_table WHERE domin_name = ? AND record_type = ?;";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("prepare sql failed");
        sqlite3_close(db);
        return NULL;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return NULL;
    }
    ret = sqlite3_bind_int(stmt, 2, record_type);

    // 执行查询
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
        LOG_ERROR("step failed when query");
        sqlite3_close(db);
        return NULL;
    }

    // 获取查询结果
    struct record_dto *dto = (struct record_dto *)malloc(sizeof(struct record_dto));
    dto->record_len = sqlite3_column_int(stmt, 1);
    dto->expire_time = sqlite3_column_int64(stmt, 2);
    memcpy(dto->record, sqlite3_column_blob(stmt, 0), dto->record_len);

    // 如果发现过期, 则删除, 但这里需要保证过期时间不为-1, -1表示永不过期
    if (dto->expire_time != -1 && dto->expire_time < time(NULL)) {
        char *delete_sql = "DELETE FROM domin_table WHERE domin_name = ? AND record_type = ?;";
        sqlite3_stmt *stmt = NULL;
        ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
        if (ret != SQLITE_OK) {
            LOG_ERROR("prepare sql failed");
            sqlite3_close(db);
            return NULL;
        }

        // 绑定参数
        ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
        if (ret != SQLITE_OK) {
            LOG_ERROR("bind param failed");
            sqlite3_close(db);
            return NULL;
        }
        ret = sqlite3_bind_int(stmt, 2, record_type);

        // 执行删除
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            LOG_ERROR("step failed when delete");
            sqlite3_close(db);
            return NULL;
        }

        // 关闭数据库
        sqlite3_close(db);

        return NULL;
    }

    // 关闭数据库
    sqlite3_close(db);


    return dto;
}

// 3.插入一条域名信息
int32 insert_domin_info(const char *domin_name, uint16 record_type, byte record[256], uint16 record_len, int32 ttl) {
    // 插入的时候先执行查询
    struct record_dto *record_dto = query_by_domin_name(domin_name, record_type);
    if (record_dto != NULL) {
        if (record_dto->expire_time == -1) {
            return SUCCESS;
        }
        if (record_dto->expire_time >= ttl + time(NULL)) {
            return SUCCESS;
        } else {
            delete_domin_info(domin_name, record_type);
        }
    }

    // 打开数据库
    sqlite3 *db = NULL;
    //char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        LOG_ERROR("open db failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 插入
    char *insert_sql = "INSERT INTO domin_table(domin_name, record_type, record, record_len, expire_time) VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("prepare sql failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int(stmt, 2, record_type);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_blob(stmt, 3, record, record_len, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int(stmt, 4, record_len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int64(stmt, 5, time(NULL) + ttl);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 执行插入
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        LOG_ERROR("step failed when insert");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    return SUCCESS;
}

// 根据域名和记录类型删除
int32 delete_domin_info(const char *domin_name, uint16 record_type) {
    // 打开数据库
    sqlite3 *db = NULL;
    //char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);

    // 删除
    char *delete_sql = "DELETE FROM domin_table WHERE domin_name = ? AND record_type = ?;";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("prepare sql failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("bind param failed");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int(stmt, 2, record_type);

    // 执行删除
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        LOG_ERROR("step failed when delete");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    return SUCCESS;
}

// 查询数据库中的所有域名信息, 传入的参数为一个二级指针, 用于存储查询结果, 返回值为查询数据的条数
int32 query_all_dns_record(struct domin_table_data **domin_table_data_array) {
    // 打开数据库
    sqlite3 *db = NULL;
    //char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        LOG_ERROR("open db failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 查询
    char *query_sql = "SELECT domin_name, record_type, record, record_len, expire_time FROM domin_table;";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        LOG_ERROR("prepare sql failed");
        sqlite3_close(db);
        return FAIL;
    }

    // 执行查询
    int32 count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }

    // 如果查询结果为空, 则直接返回
    if (count == 0) {
        sqlite3_close(db);
        return 0;
    }

    // 申请内存
    *domin_table_data_array = (struct domin_table_data *)malloc(sizeof(struct domin_table_data) * count);
    // 将查询结果存入数组
    sqlite3_reset(stmt);
    int32 i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        strcpy((*domin_table_data_array)[i].domin_name, (const char*)sqlite3_column_text(stmt, 0));
        (*domin_table_data_array)[i].record_type = sqlite3_column_int(stmt, 1);
        (*domin_table_data_array)[i].record_len = sqlite3_column_int(stmt, 3);
        (*domin_table_data_array)[i].expire_time = sqlite3_column_int64(stmt, 4);
        memcpy((*domin_table_data_array)[i].record, sqlite3_column_blob(stmt, 2), (*domin_table_data_array)[i].record_len);
        i++;
    }

    // 关闭数据库
    sqlite3_close(db);

    return count;
}