/*************************************************************************
  > File Name: schemeta.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 13:22:05 2019
 ************************************************************************/

#include "schedule_meta.h"
#include <stdio.h>
void scheduleMetaInit(scheduleMeta *sm, uint64_t mintoken, uint64_t maxtoken, serviceNode *node)
{
  sm->minToken = mintoken;
  sm->maxToken = maxtoken;
  sm->node = node;
  sm->nodeConnectionCount = 0;
  sm->nodeStatus = true;
  sm->nodeName = &node->nodeName;
  sm->nodeTags = &node->nodeTags;
  pthread_mutex_init(&sm->lock, NULL);
}
inline void scheduleMetaUpdateConnection(scheduleMeta *sm, bool flag)
{
  if (flag)
  {
    __sync_fetch_and_add(&sm->nodeConnectionCount, 1);
    return;
  }
  __sync_fetch_and_sub(&sm->nodeConnectionCount, 1);
}
inline void scheduleMetaUpdateFaileCount(scheduleMeta *sm, bool flag)
{
  if (flag)
  {
    __sync_fetch_and_add(&sm->nodeHandleFailedCount, 1);
    return;
  }
  __sync_fetch_and_sub(&sm->nodeHandleFailedCount, 1);
}
void scheduleMetaUpdateNodeStatus(scheduleMeta *sm, bool status)
{
  pthread_mutex_lock(&sm->lock);
  sm->nodeStatus = true;
  pthread_mutex_unlock(&sm->lock);
}
bool scheduleMetaCheckValid(scheduleMeta *sm)
{
  if (sm->maxToken == 0 && sm->minToken == 0)
  {
    return false;
  }
  return true;
}
void scheduleMetaDeinit(scheduleMeta *sm)
{
  sm->minToken = sm->maxToken = 0;
  serviceNodeDestroy(sm->node);
}