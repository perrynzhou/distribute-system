/*************************************************************************
  > File Name: tcp_server.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Fri 12 Apr 2019 04:55:57 PM CST
 ************************************************************************/

#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H
#include "request_token.h"
#include <stdint.h>
struct tcpServer {
    int port;
    int backlog;
    int sockfd;
    int bucket;
    struct tokenInfo *tokens;
    uint8_t *store;
};
int tcpServerInit(struct tcpServer* ts, int port, int backlog,int bucket);
void tcpServerRun(struct tcpServer* ts);
void tcpServerDeinit(struct tcpServer* ts);
#endif
