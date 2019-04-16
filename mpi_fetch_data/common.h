/*************************************************************************
  > File Name: common.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Sat 13 Apr 2019 10:56:42 AM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
#include <cstdint>
struct Token {
  int rank_;
  uint64_t start_;
  uint64_t end_;
  int bucket_;
  bool is_sync_;
};
struct Message {
  int rank_;
  uint64_t data_;
  int flag_;// -1  is unknow,0 is handshake;1 is write,2,is read,3 is close
};
struct Metric {
    int type_;
    uint64_t count_;
};
enum RequestOpType {
     HandshakeType=0,
     ReadType,
     WriteType,
     CloseType,
};
int initTcpSocket(const char *addr,int port, int backlog);
int initTcpClient(const char *addr,int port);
void setBit(uint32_t *a, int n);
void clsBit(uint32_t *a, int n);
int judgeBit(uint32_t *a, int n);
uint32_t randInt();
#endif
