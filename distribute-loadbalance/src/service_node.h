/*************************************************************************
  > File Name: service_node.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 12:41:29 2019
 ************************************************************************/

#ifndef _SERVICE_NODE_H
#define _SERVICE_NODE_H
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
typedef int (*readCb)(int cfd);
typedef int (*writeCb)(int cfd, void *ptr, size_t len);
typedef int (*exceptionCb)(int cfd, const char *msg,size_t len);
enum serviceNodeErrorCode
{
  invalidCbError = -1,
  invalidSockError = -2
};
typedef struct serviceNode
{
  struct string  *tag;
  int uid;
  int threadCount;
  pthread_t *thds;
  int sock;
  bool isStop;
  readCb readcb;
  writeCb writecb;
  exceptionCb exceptioncb;
} serviceNode;
serviceNode *serviceNodeCreate(const char *tag,int port, int threads);
void serviceNodeInit(serviceNode *sn,const char *tag, int port, int threads);
void serviceNodeSetCb(serviceNode *sn, readCb readcb, writeCb writecb, exceptionCb exceptioncb);
int serviceNodeRun(serviceNode *sn);
void serviceNodeStop(serviceNode *sn);
void serviceNodeDeinit(serviceNode *sn);
void serviceNodeDestroy(serviceNode *sn);
#endif
