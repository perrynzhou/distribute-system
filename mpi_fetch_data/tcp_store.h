/*************************************************************************
  > File Name: tcp_store.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 日  4/14 14:00:49 2019
 ************************************************************************/

#ifndef _TCP_STORE_H
#define _TCP_STORE_H
#include "common.h"
#include <stdint.h>
typedef struct
{
   int sockfd;
   int bucket;
   token_t *tokens;
   metric_t *metrics;
   int    *cache;
} tcp_store_t;
int tcp_store_init(tcp_store_t *ts, const char *addr, int port, int backlog, int bucket);
int tcp_store_run(tcp_store_t *ts);
void tcp_store_deinit(tcp_store_t *ts);
#endif
