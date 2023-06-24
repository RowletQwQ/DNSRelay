#include "db.h"
#include "sqlite3.h"
#include <stddef.h>
#include <stdio.h>


// 1.初始化数据库, 建表
int32 init_db() {
    // 先打开数据库, 如果数据库不存在, 则创建数据库
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "open db failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 创建表
    char *create_table_sql = "CREATE TABLE domin_table (domin_name TEXT PRIMARY KEY, record_type INTEGER, record BLOB, expire_time INTEGER);";
    ret = sqlite3_exec(db, create_table_sql, NULL, 0, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "create table failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    return SUCCESS;
}

// 2.根据域名查询, 结果为NULL表示查询失败
struct record_dto *query_by_domin_name(const char *domin_name) {
    // 打开数据库
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "open db failed\n");
        sqlite3_close(db);
        return NULL;
    }

    // 查询
    char *query_sql = "SELECT * FROM domin_table WHERE domin_name = ?;";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "prepare sql failed\n");
        sqlite3_close(db);
        return NULL;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return NULL;
    }

    // 执行查询
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
        fprintf(stdout, "step failed\n");
        sqlite3_close(db);
        return NULL;
    }

    // 获取查询结果中的expire_time
    int64 expire_time = sqlite3_column_int64(stmt, 3);
    // 如果过期了, 就在数据库中删除这条记录并返回NULL
    if (expire_time <= time(NULL)) {
        char *delete_sql = "DELETE FROM domin_table WHERE domin_name = ?;";
        ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
        if (ret != SQLITE_OK) {
            fprintf(stdout, "prepare sql failed\n");
            sqlite3_close(db);
            return NULL;
        }
        // 绑定参数
        ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
        if (ret != SQLITE_OK) {
            fprintf(stdout, "bind param failed\n");
            sqlite3_close(db);
            return NULL;
        }
        // 执行删除
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            fprintf(stdout, "step failed\n");
            sqlite3_close(db);
            return NULL;
        }
        // 关闭数据库
        sqlite3_close(db);
        return NULL;
    }

    // 获取查询结果
    struct record_dto *dto = (struct record_dto *)malloc(sizeof(struct record_dto));
    dto->record_dto = sqlite3_column_int(stmt, 1);
    memcpy(dto->record, sqlite3_column_blob(stmt, 2), sqlite3_column_bytes(stmt, 2));

    // 关闭数据库
    sqlite3_close(db);

    return dto;
}

// 3.插入一条域名信息
int32 insert_domin_info(const char *domin_name, uint16 record_type, byte record[256], int32 ttl) {
    // 打开数据库
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "open db failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 插入, 先计算过期时间
    int64 expire_time = time(NULL) + ttl;
    char *insert_sql = "INSERT INTO domin_table (domin_name, record_type, record, expire_time) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "prepare sql failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 绑定参数
    ret = sqlite3_bind_text(stmt, 1, domin_name, -1, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int(stmt, 2, record_type);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_blob(stmt, 3, record, 256, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int64(stmt, 4, expire_time);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 执行插入
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stdout, "step failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    return SUCCESS;
}