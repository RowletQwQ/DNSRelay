#include "logger.h"
#include "parsedata.h"
#include <stdio.h>

int parse_to_domain(char * message,char * domain,int len){
    int i = 0, j = 0;
    while (message[i] != 0) {
        if(i > len){
            write_log(LOG_LEVEL_FATAL,"parse_to_domain error");
            exit(1);
        }
        int len = message[i];
        for (int k = 0; k < len; k++) {
            domain[j++] = message[++i];
        }
        domain[j++] = '.';
        i++;
    }
    domain[j - 1] = '\0'; // 去除最后一个'.'
    
    return i + 4;
}

int parse_to_domains(char * message,char * domains[]){
    struct dnsheader *dnshdr = (struct dnsheader *)message;
    int qdcount = ntohs(dnshdr->qdcount);

    domains = (char **)malloc(sizeof(char *) * qdcount);
    
    char * domain = NULL;
    
    
    // 从12字节后解析，跳过头部
    for(int i = 0,offset = 12;i < qdcount;i++){
        domain = (char *)malloc(sizeof(char) * 256);
        
        if(domain == NULL){
            write_log(LOG_LEVEL_FATAL,"malloc failed");
            exit(1);
        }

        offset += parse_to_domain(buf + offset, domain, 256);
        write_log(LOG_LEVEL_INFO,"query name: %s",domain);
        domains[i] = domain;
    }

    write_log(LOG_LEVEL_INFO,"query name: %s",dns_name);
    return qdcount;
}

void parse_add_dnsans(char *ip[],int len,char * message){
    return;
}