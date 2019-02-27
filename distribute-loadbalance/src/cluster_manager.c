/*************************************************************************
  > File Name: cluster_manager.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 18:07:38 2019
 ************************************************************************/

#include "cluster_manager.h"
#include "dict.h"
#include "log.h"
#include "conf.h"
#include "array.h"
#include "util.h"
#include "hashkit.h"
#include <stdio.h>
#include <stdint.h>
#define CLUSTER_MANAGER_NODES_CAP (64)
void nodeConnectionInfoInit(nodeConnectionInfo *nci, const char *addr, int port)
{
  stringInitWithData(&nci->addr, addr);
  nci->port = port;
  initUdpSocket(addr, port, &nci->info);
}
void nodeConnectionInfoDeinit(nodeConnectionInfo *nci)
{
  stringDeinit(&nci->addr);
  if (nci->info.sock != -1)
  {
    close(nci->info.sock);
  }
}
void clusterManagerInit(clusterManager *cm, const char *addr, int maxNodeSize, const char *path)
{
  if (maxNodeSize > CLUSTER_MANAGER_NODES_CAP)
  {
    logError("over maxsize(%d) for nodes!!!", CLUSTER_MANAGER_NODES_CAP);
    return;
  }
  logInit(LOG_INFO_LEVEL, NULL);
  struct conf *f = confCreate(path, false);
  if (f != NULL)
  {
    confDump(f);
  }
  uint32_t n = arrayGetSize(&f->pool);
  if (n == 0)
  {
    confDestroy(f);
    return;
  }
  cm->dt = dictCreate(0, (hashCb)&hash_fnv1a_64, (destroyCb)&serviceNodeDestroy);
  cm->nodeSize = n;
  uint64_t u32Max = UINT32_MAX;
  uint64_t perToken = u32Max / n;
  uint64_t restToken = u32Max % n;
  uint64_t minToken = 0, maxToken = 0;
  //scheduleMeta *meta = (scheduleMeta *)calloc(n, sizeof(*meta));
  cm->meta = (scheduleMeta **)calloc(n, sizeof(scheduleMeta *));
  cm->udpVec = vectorCreate(n * 2, NULL, NULL);
  nodeConnectionInfo *ncies = (nodeConnectionInfo *)calloc(n, sizeof(nodeConnectionInfo));
  cm->ctx = ncies;
  for (int i = 0; i < n; i++)
  {
    struct confPool *pool = (struct confPool *)arrayGet(&f->pool, i);
    const char *name = (const char *)pool->nodeName.data;
    const char *tags = (const char *)pool->nodeTags.data;
    const char *serviceAddr = (const char *)pool->nodeAddr.data;
    int port = pool->nodePort;
    int backlog = pool->nodeTcpBacklog;
    int threads = pool->nodeWorkerThreads;
    int timeout = pool->nodeTcpTimeout;
    serviceNode *sn = serviceNodeCreate(name, tags, threads);
    serviceNodeSetSocketInfo(sn, serviceAddr, port, timeout, backlog);
    serviceNodeSetClusterAddr(sn, addr);

    if (i != (n - 1))
    {
      minToken = i * perToken;
      maxToken = (i + 1) * perToken - 1;
    }
    else
    {
      minToken = maxToken + 1;
      maxToken = u32Max - 1;
    }
    cm->meta[i] = (scheduleMeta *)calloc(1, sizeof(scheduleMeta));
    nodeConnectionInfoInit(&ncies[i], (const char *)sn->nodeAddr.data, sn->reportStatusPort);
    scheduleMetaInit(cm->meta[i], minToken, maxToken, sn);
    dictAdd(cm->dt, cm->meta[i]->nodeName, cm->meta[i]);
    logInfo(LOG_INFO_LEVEL, "server info:name=%s,tags=%s,mintoken=%ld,maxtoken=%ld", (char *)cm->meta[i]->nodeName->data, (char *)cm->meta[i]->nodeTags->data, cm->meta[i]->minToken, cm->meta[i]->maxToken);
  }
  if (f != NULL)
  {
    confDestroy(f);
  }
}
void clusterManagerDeinit(clusterManager *cm)
{
  if (cm->dt != NULL)
  {
    dictDestroy(cm->dt);
    int n = vectorSize(cm->udpVec);
    cm->dt = NULL;
    for (int i = 0; i < cm->nodeSize; i++)
    {
      nodeConnectionInfo *info = (nodeConnectionInfo *)vectorGet(cm->udpVec, i);
      if (info != NULL)
      {
        nodeConnectionInfoDeinit(info);
      }
    }
    free(cm->ctx);
  }
}
#ifdef TEST
int main()
{
  logInit(LOG_INFO_LEVEL, NULL);
  clusterManager cm;
  clusterManagerInit(&cm, "127.0.0.1:10091", 16, "./conf.yaml");
  clusterManagerDeinit(&cm);
  return 0;
}
#endif