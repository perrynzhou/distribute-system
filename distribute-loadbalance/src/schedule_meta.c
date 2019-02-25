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
}