/*************************************************************************
  > File Name: cluster_manger.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 18:04:53 2019
 ************************************************************************/

#ifndef _CLUSTER_MANAGER_H
#define _CLUSTER_MANAGER_H
#include "schedule_meta.h"
#include "dict.h"
#include "vector.h"
#include "util.h"
// used udp for heartbeat
typedef struct nodeConnectionInfo
{
  struct string addr;
  int port;
  UdpInfo info;
} nodeConnectionInfo;
typedef struct clusterManager
{
  dict *dt;
  vector *udpVec; // save udp service_node addr and port
  scheduleMeta **meta;
  void  *ctx;
  int accpetRequestSock; //tcp
  int heartBeatSock;     // udp
  int nodeSize;
  int nodeCap; // max node in cluster
} clusterManager;

void nodeConnectionInfoInit(nodeConnectionInfo *nci,const char *addr,int port);
void nodeConnectionInfoDeinit(nodeConnectionInfo *nci);

void clusterManagerInit(clusterManager *cm, const char *addr, int maxNodeSize, const char *path);
void clusterManagerRun(clusterManager *cm);
void clusterManagerAddNode(clusterManager *cm, const char *srvAddr);
void clusterManagerDelNode(clusterManager *cm, const char *srvAddr);
void clusterMangerPrint(clusterManager *cm);
void clusterManagerDeinit(clusterManager *cm);
#endif
