/*************************************************************************
  > File Name: service_node.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 13:27:19 2019
 ************************************************************************/
#include "service_node.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#define SERVICE_NODE_MAX_BACKLOG (1024)
serviceNode *serviceNodeCreate(const char *name,const char *tags, int threads)
{
  serviceNode *node = (serviceNode *)calloc(1, sizeof(*node));
  assert(node != NULL);
  serviceNodeInit(node, name,tags, threads);
  return node;
}
void serviceNodeSetClusterInfo(serviceNode *sn,const char *caddr,int port,int timeout) {
    stringInitWithData(&sn->clusterAddr,caddr);
    sn->clusterHeartbeatPort = port;
    sn->reportStatusTimeout = timeout;
}
void serviceNodeSetSocketInfo(serviceNode *sn,const char *addr,int port,int timeout,int backlog){
 stringInitWithData(&sn->nodeAddr,addr);
  int backlog_ = (backlog<SERVICE_NODE_MAX_BACKLOG)?SERVICE_NODE_MAX_BACKLOG:backlog;
  sn->sock = initTcpSocket(port, backlog_);
  sn->timeout = timeout;
}
inline void serviceNodeSetClusterAddr(serviceNode *sn,const char *clusterAddr) {
  stringInitWithData(&sn->clusterAddr,clusterAddr);
}
void serviceNodeInit(serviceNode *sn,const char *name, const char *tags,int threads)
{
  stringInitWithData(&sn->nodeName,name);
  stringInitWithData(&sn->nodeTags,tags);
  sn->thds = (pthread_t *)calloc(threads, sizeof(pthread_t));
  sn->isStop = false;
}
void serviceNodeSetCb(serviceNode *sn, readCb readcb, writeCb writecb, exceptionCb exceptioncb)
{
  sn->exceptioncb = exceptioncb;
  sn->readcb = readcb;
  sn->writecb = writecb;
}
static void serviceNodeAcceptConnection(void *arg)
{
  serviceNode *node = (serviceNode *)arg;
  pthread_t pid = pthread_self();
  fprintf(stdout, "%d worker started at %ld\n", node->uid, pid);

  while (!node->isStop)
  {
    int cfd = accept(node->sock, 0, 0);
    if (cfd != -1)
    {
      if (!node->readcb(cfd))
      {
        node->writecb(cfd, "", 1);
      }
      else
      {
        node->exceptioncb(cfd, "", 1);
      }
    }
  }
  fprintf(stdout, "%d worker stoped at %ld\n", node->uid, pid);
}
int serviceNodeRun(serviceNode *sn)
{
  for (int i = 0; i < sn->threadCount; i++)
  {
    pthread_create(&sn->thds[i], NULL, (void *)&serviceNodeAcceptConnection, (void *)sn);
  }
  for (int i = 0; i < sn->threadCount; i++)
  {
    pthread_join(sn->thds[i], NULL);
  }
}
void serviceNodeStop(serviceNode *sn)
{
  sn->isStop = true;
}
void serviceNodeDeinit(serviceNode *sn)
{
  if (sn->sock != -1)
  {
    close(sn->sock);
  }
  free(sn->thds);
  stringDeinit(&sn->nodeTags);
  stringDeinit(&sn->nodeName);
  sn->thds = NULL;
}
void serviceNodeDestroy(serviceNode *sn)
{
  serviceNodeDeinit(sn);
  if (sn != NULL)
  {
    free(sn);
    sn = NULL;
  }
}