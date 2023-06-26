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
        fprintf(stdout, "create table failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 关闭数据库
    sqlite3_close(db);

    return SUCCESS;
}

// 2.根据域名和记录类型进行查询, 结果为NULL表示查询失败
struct record_dto *query_by_domin_name(const char *domin_name, uint16 record_type) {
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
    char *query_sql = "SELECT record, record_len, expire_time FROM domin_table WHERE domin_name = ? AND record_type = ?;";
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
    ret = sqlite3_bind_int(stmt, 2, record_type);

    // 执行查询
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
        fprintf(stdout, "step failed\n");
        sqlite3_close(db);
        return NULL;
    }

    // 获取查询结果
    struct record_dto *dto = (struct record_dto *)malloc(sizeof(struct record_dto));
    dto->record_len = sqlite3_column_int(stmt, 1);
    dto->expire_time = sqlite3_column_int64(stmt, 2);
    memcpy(dto->record, sqlite3_column_blob(stmt, 0), dto->record_len);

    // 如果发现过期, 则删除
    if (dto->expire_time < time(NULL)) {
        char *delete_sql = "DELETE FROM domin_table WHERE domin_name = ? AND record_type = ?;";
        sqlite3_stmt *stmt = NULL;
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
        ret = sqlite3_bind_int(stmt, 2, record_type);

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

    // 关闭数据库
    sqlite3_close(db);


    return dto;
}

// 3.插入一条域名信息
int32 insert_domin_info(const char *domin_name, uint16 record_type, byte record[256], uint16 record_len, int32 ttl) {
    // 打开数据库
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int32 ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "open db failed\n");
        sqlite3_close(db);
        return FAIL;
    }

    // 插入
    char *insert_sql = "INSERT INTO domin_table(domin_name, record_type, record, record_len, expire_time) VALUES(?, ?, ?, ?, ?);";
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
    ret = sqlite3_bind_blob(stmt, 3, record, record_len, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int(stmt, 4, record_len);
    if (ret != SQLITE_OK) {
        fprintf(stdout, "bind param failed\n");
        sqlite3_close(db);
        return FAIL;
    }
    ret = sqlite3_bind_int64(stmt, 5, time(NULL) + ttl);
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