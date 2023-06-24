#include "logger.h"

int main(){
    init_log("test.log",LOG_LEVEL_DEBUG);
    int a = 1;
    LOG_DEBUG("test debug %d", a);
    LOG_INFO("test info %d", a);
    LOG_WARN("test warn %d", a);
    LOG_ERROR("test error %d", a);
    return 0;
}