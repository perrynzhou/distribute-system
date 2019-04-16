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
class TcpStore
{
  string addr_;
  int port_;
  int sockfd_;
  int backlog_;
  int bucket_;
  struct Token *tokens_;
  uint64_t    *countor_;//record each node read/write times
  //prepare resource befor run
  int Prepare();

public:
  TcpStore(const char *addr, int port, int bucket);
  void Run();
  ~TcpStore();
};
#endif
