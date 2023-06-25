#include <stdio.h>
#include <stdlib.h>

#include "db.h"
#include "userfile.h"
#include "logger.h"


int main(){
    init_log("log.txt", LOG_LEVEL_DEBUG, 1, NULL);
    init_read_file("userfile.txt");
    struct domin_table_data buffer[1024] = {0};
    int len = read_data(buffer, 1024);
    printf("len:%d\n", len);
    init_write_file("userfile.txt");
    write_data(buffer, len);
    return 0;
}