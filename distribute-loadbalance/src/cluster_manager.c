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
void clusterManagerInit(clusterManager *cm, const char *addr, const char *path)
{
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
  scheduleMeta *meta = (scheduleMeta *)calloc(n, sizeof(*meta));
  cm->meta = meta;
  for (int i = 0; i < n; i++)
  {
    struct confPool *pool = (struct confPool *)arrayGet(&f->pool, i);
    const char *name = (const char *)pool->name.data;
    const char *tags = (const char *)pool->tags.data;
    const char *serviceAddr = (const char *)pool->addr.data;
    int port = pool->port;
    int backlog = pool->backlog;
    int threads = pool->threads;
    int timeout = pool->timeout;
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
      minToken = maxToken+1;
      maxToken = u32Max - 1;
    }
    scheduleMetaInit(&meta[i], minToken, maxToken, sn);
    dictAdd(cm->dt, &meta[i].nodeName, &meta);
    logInfo(LOG_INFO_LEVEL, "server info:name=%s,tags=%s,mintoken=%ld,maxtoken=%ld", (char *)meta[i].nodeName->data, (char *)meta[i].nodeTags->data, meta[i].minToken, meta[i].maxToken);
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
    cm->dt = NULL;
  }
}
#ifdef TEST
int main()
{
  logInit(LOG_INFO_LEVEL, NULL);
  clusterManager cm;
  clusterManagerInit(&cm, "127.0.0.1:10091", "./conf.yaml");
  clusterManagerDeinit(&cm);
  return 0;
}
#endif