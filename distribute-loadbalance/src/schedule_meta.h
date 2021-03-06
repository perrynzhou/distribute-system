/*************************************************************************
  > File Name: schedule_meta.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 13:21:59 2019
 ************************************************************************/

#ifndef _SCHEDULE_META_H
#define _SCHEDULE_META_H
#include "service_node.h"
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
typedef struct scheduleMeta
{
  uint64_t minToken;
  uint64_t maxToken;
  struct string *nodeName;
  struct string *nodeTags;
  uint64_t nodeConnectionCount;
  uint64_t nodeHandleFailedCount;
  int nodeWeight;
  bool nodeStatus;
  serviceNode *node;
  pthread_mutex_t lock;
} scheduleMeta;
void scheduleMetaInit(scheduleMeta *sm, uint64_t mintoken, uint64_t maxtoken, serviceNode *node);
bool scheduleMetaCheckValid(scheduleMeta *sm);
void scheduleMetaUpdateNodeStatus(scheduleMeta *sm, bool status);
void scheduleMetaUpdateConnection(scheduleMeta *sm, bool flag);
void scheduleMetaUpdateFaileCount(scheduleMeta *sm, bool flag);
void scheduleMetaDeinit(scheduleMeta *sm);
#endif
