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
void clusterMangerCreate(clusterManager *cm,const char *addr,const char *path)
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
  cm->dt = dictCreate(0, &hash_fnv1a_64, &serviceNodeDestroy);
  cm->nodeSize = n;
  uint64_t perToken = UINT32_MAX/n;
  uint64_t restToken = UINT32_MAX%n;
  uint64_t minToken=0,maxToken = 0;
  scheduleMeta *meta=(scheduleMeta *)calloc(n,sizeof(*meta));
  cm->meta = meta;
  for (int i = 0; i < n; i++)
  {
    struct confPool *pool = (struct confPool *)arrayGet(&f->pool, i);
    const char *name = (const char *)pool->name.data;
    const char *tags =(const char *)pool->tags.data;
    const char *serviceAddr = (const char *)pool->addr.data;
    int port = pool->port;
    int backlog = pool->backlog;
    int threads = pool->threads;
    int timeout = pool->timeout;
    serviceNode *sn = serviceNodeCreate(name,tags,threads);
    serviceNodeSetSocketInfo(sn,serviceAddr,port,timeout,backlog);
    serviceNodeSetClusterAddr(sn,addr);
    uint64_t nextToken = (i+1)*perToken;
    maxToken = (i<n-1)?nextToken:(nextToken+restToken);
    scheduleMetaInit(&meta[i],minToken,maxToken,sn);
    minToken= maxToken+1;
    dictAdd(cm->dt,&meta[i].nodeName,&meta);
    logInfo(LOG_INFO_LEVEL,"server info:name=%s,tags=%s,mintoken=%d,maxtoken=%d",meta[i].nodeName->data,meta[i].nodeTags->data,mata[i].minToken,meta[i].maxToken);
  }
  if (f != NULL)
  {
    confDestroy(f);
  }
}
void clusterManagerDeinit(clusterManager *cm){
  if(cm->dt !=NULL) {
    dictDestroy(cm->dt);
    cm->dt = NULL;
  }
}