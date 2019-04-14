/*************************************************************************
  > File Name: common.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Sat 13 Apr 2019 10:56:42 AM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
#include <stdbool.h>
#include <stdint.h>
struct token {
  int rank;
  uint64_t start;
  uint64_t end;
  bool is_sync;
};
struct request {
  int rank;
  int data;
  int flag;// -1  is unknow,0 is handshake;1 is write,2,is read,3 is close
};
struct metric {
    int type;
    uint64_t count;
};
enum reqest_op_type {
     handshake_type=0,
     read_type,
     write_type,
     close_type,
};
int init_tcp_socket(const char *addr,int port, int backlog);
void set_bit(uint32_t *a, int n);
void cls_bit(uint32_t *a, int n);
int is_exists(uint32_t *a, int n);
uint32_t rand_int();
#endif
