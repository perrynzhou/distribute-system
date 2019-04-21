/*************************************************************************
  > File Name: common.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Sat 13 Apr 2019 10:56:42 AM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
typedef struct
{
  int rank;
  uint64_t start;
  uint64_t end;
  int bucket;
  bool is_sync;
} token_t;
typedef struct
{
  int rank;
  uint64_t data;
  int flag; // -1  is unknow,0 is handshake;1 is write,2,is read,3 is close
} message_t;
typedef struct
{
  int rank;
  uint64_t count;
} metric_t;
typedef struct {
  pthread_t thread;
  int id;
}mpi_thread_t;
typedef enum
{
  handshake_type = 0,
  read_type,
  write_type,
  close_type,
} request_op_type;
int init_tcp_socket(const char *addr, int port, int backlog);
int init_tcp_client(const char *addr, int port);
void set_bit(uint32_t *a, int n);
void cls_bit(uint32_t *a, int n);
int judge_bit(uint32_t *a, int n);
uint32_t rand_int();
#endif
