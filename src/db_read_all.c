#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "db.h"

#define file_name "db_data.txt"

// 将数据库中的所有数据读取到一个文件中
int main() {
    // 先调用db中的query_all_dns_record函数
    struct domin_table_data *data = NULL;
    int32 cnt = query_all_dns_record(&data);

    // 将数据写入文件 (覆盖写入)
    FILE *fp = fopen(file_name, "w");
    assert(fp != NULL);

    for (int32 i = 0; i < cnt; i++) {
        fprintf(fp, "%s %d %d %d %ld\n", data[i].domin_name, data[i].record_type, data[i].record_len, data[i].expire_time, data[i].record);
    }

    fclose(fp);
    
    return 0;
}