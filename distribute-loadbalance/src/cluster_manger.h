/*************************************************************************
  > File Name: cluster_manger.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 18:04:53 2019
 ************************************************************************/

#ifndef _CLUSTER_MANGER_H
#define _CLUSTER_MANGER_H
#include "schedule_meta.h"
#include "dict.h"
typedef struct clusterManager
{
  dict *dt;
  int nodeSize;
};
void clusterMangerInit(clusterManager *cm);
void clusterManagerAddNode();
void clusterManagerDelNode();
void clusterMangerPrint(clusterManager *cm);
void clusterManagerDeinit(clusterManager *cm);
#endif
