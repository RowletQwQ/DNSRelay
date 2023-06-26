#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include "setting.h"
int main(int argc, char *argv[])
{
    say_hello();
    parse_args(argc,argv);
    //打印设置
    char *ip = get_dns_server_ip();
    printf("debug_level: %d\n",get_debug_level());
    printf("dns_server_ip: %s\n",ip);
    printf("user_file: %s\n",get_user_file());
    printf("log_file: %s\n",get_log_file());
    free(ip);
    return 0;
}