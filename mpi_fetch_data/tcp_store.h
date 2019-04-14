/*************************************************************************
  > File Name: tcp_store.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 日  4/14 14:00:49 2019
 ************************************************************************/

#ifndef _TCP_STORE_H
#define _TCP_STORE_H
#include <stdint.h>
struct tcp_store {
    int port;
    int sockfd;
    int bucket;
    struct token *tokens;
    uint32_t *store;
};
int tcp_store_init(struct tcp_store* ts, int port, int backlog,int bucket);
void tcp_store_run(struct tcp_store* ts);
void tcp_store_deinit(struct tcp_store* ts);
#endif
