/*************************************************************************
  > File Name: service_node.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 12:41:29 2019
 ************************************************************************/

#ifndef _SERVICE_NODE_H
#define _SERVICE_NODE_H
#include "cstring.h"
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
  struct string  nodeTags;
  struct string  nodeName;
  struct string nodeAddr;
  struct string  clusterAddr;
  int  clusterHeartbeatPort;
  int uid;
  int threadCount;
  int  reportStatusPort;
  int reportStatusTimeout;
  pthread_t *thds;
  int sock;
  bool isStop;
  int timeout;
  uint64_t tokenCount;
  readCb readcb;
  writeCb writecb;
  exceptionCb exceptioncb;
} serviceNode;
serviceNode *serviceNodeCreate(const char *name,const char *tag,int threads);
void serviceNodeSetClusterInfo(serviceNode *sn,const char *caddr,int port,int timeout);
void serviceNodeInit(serviceNode *sn,const char *name,const char *tags, int threads);
void serviceNodeSetSocketInfo(serviceNode *sn,const char *addr,int port,int timeout,int backlog);
void serviceNodeSetClusterAddr(serviceNode *sn,const char *clusterAddr);
void serviceNodeSetCb(serviceNode *sn, readCb readcb, writeCb writecb, exceptionCb exceptioncb);
int serviceNodeRun(serviceNode *sn);
void serviceNodeStop(serviceNode *sn);
void serviceNodeDeinit(serviceNode *sn);
void serviceNodeDestroy(serviceNode *sn);
#endif
