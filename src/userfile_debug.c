#include <stdio.h>
#include <stdlib.h>

#include "db.h"
#include "userfile.h"
#include "logger.h"


int main(){
    init_log("log.txt", LOG_LEVEL_DEBUG, 1, NULL);
    init_read_file("userfile.txt");
    struct domin_table_data *buffer = NULL;
    int len = read_data(&buffer);
    printf("len:%d\n", len);
    init_write_file("userfile.txt");
    write_data(buffer, len);
    free(buffer);
    return 0;
}