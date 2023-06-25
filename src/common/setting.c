#include "setting.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

typedef struct setting
{
    int debug_level;// 调试等级
    int dns_type;// DNS服务器类型
    struct in_addr dns_server_4;// DNS服务器地址
    struct in6_addr dns_server_6;// DNS服务器地址
    char user_file[256];// 用户自定义文件
    char log_file[256];// 日志文件
}setting_t;
static setting_t setting;

//Hello信息,包括一些版权声明
void say_hello(){
    printf("DNS Relay/Proxy Server, Version 1.0.0\n");
    printf("Author: RowletQwQ,JollyCorivuG,zjsycwmrbig\n");
    printf("Build time: %s %s\n",__DATE__,__TIME__);
}

// 帮助信息输出
void say_help(){
    printf("Usage: dnsrelay [-d <debug_level>] [-s <dns-server-ip>] [-c <config-file>] [-h] [-l <log-file>]\n");
    printf("Options:\n");
    printf("  -d <debug_level>        Set the debug level, 0-3, default 0\n");
    printf("  -s <dns-server-ip>      Set the dns server ip, default 114.114.114.114\n");
    printf("  -c <config-file>        Set the config file, default dnsrelay.txt\n");
    printf("  -h                      Show this help message\n");
    printf("  -l <log-file>           Set the log file, default dnsrelay.log\n");
}

// 解析命令行参数
void parse_args(int argc,char *argv[]){
    int opt;
    memset(&setting,0,sizeof(setting));
    // 设置默认值
    setting.debug_level = 0;
    setting.dns_type = AF_INET;
    inet_pton(AF_INET,"114.114.114.114",&setting.dns_server_4);
    strcpy(setting.user_file,"dnsrelay.txt");
    strcpy(setting.log_file,"dnsrelay.log");
    // 解析命令行参数
    while((opt = getopt(argc,argv,"d:s:c:l:h")) != -1){
        switch(opt){
            case 'd':
                setting.debug_level = (atoi(optarg) > 3 || atoi(optarg) < 0) ? 0 : atoi(optarg);
                break;
            case 's':
                if(inet_pton(AF_INET,optarg,&setting.dns_server_4) == 1){
                    setting.dns_type = AF_INET;
                }else if(inet_pton(AF_INET6,optarg,&setting.dns_server_6) == 1){
                    setting.dns_type = AF_INET6;
                }else{
                    printf("Invalid dns server ip\n");
                    say_help();
                    exit(1);
                }
                break;
            case 'c':
                strcpy(setting.user_file,optarg);
                break;
            case 'h':
                say_help();
                exit(0);
            case 'l':
                strcpy(setting.log_file,optarg);
                break;
            default:
                printf("Invalid option\n");
                say_help();
                exit(1);
        }
    }
    
}

int get_debug_level(){
    return setting.debug_level;
}


struct my_dns_addr get_dns_server(){
    struct my_dns_addr addr = {0};
    if(setting.dns_type == AF_INET){
        memset(addr,0,sizeof(struct my_dns_addr));
        addr->family = AF_INET;
        memcpy(&addr->u,&setting.dns_server_4,sizeof(struct in_addr));
    }else if(setting.dns_type == AF_INET6){ 
        memset(addr,0,sizeof(struct my_dns_addr));
        addr->family = AF_INET6;
        memcpy(&addr->u,&setting.dns_server_6,sizeof(struct in6_addr));
    }
    return addr;
}

char *get_dns_server_ip(){
    char *ip = (char*)malloc(64);
    memset(ip,0,64);
    if(setting.dns_type == AF_INET){
        inet_ntop(AF_INET,&setting.dns_server_4,ip,64);
    }else if(setting.dns_type == AF_INET6){
        inet_ntop(AF_INET6,&setting.dns_server_6,ip,64);
    }
    return ip;
}
char *get_user_file(){
    return setting.user_file;
}

char *get_log_file(){
    return setting.log_file;
}

int get_dns_type(){
    return setting.dns_type;
}