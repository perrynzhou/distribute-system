/*************************************************************************
  > File Name: tcp_store.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: 日  4/14 14:00:49 2019
 ************************************************************************/

#ifndef _TCP_STORE_H
#define _TCP_STORE_H
#include <cstdint>
#include <iostream>
using namespace std;
class TcpStore {
  int backlog_ = 1024;
  int bucket_;
  int port_;
  int sockfd_;
  struct Token *tokens_;
  uint32_t *store_;

public:
  TcpStore(const char *addr, int port, int bucket);
  void Run();
  ~TcpStore();
};
#endif
