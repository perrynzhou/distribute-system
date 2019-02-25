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
#define SERVICE_Node_MAX_BACKLOG (1024)
serviceNode *serviceNodeCreate(const char *tag, int port, int threads)
{
  serviceNode *node = (serviceNode *)calloc(1, sizeof(*node));
  assert(node != NULL);
  serviceNodeInit(node, tag, port, threads);
  return node;
}
void serviceNodeInit(serviceNode *sn, const char *tag, int port, int threads)
{
  sn->tag = strdup(tag);
  sn->sock = initSocket(port, SERVICE_Node_MAX_BACKLOG);
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
  free(sn->tag);
  sn->thds = NULL;
  sn->tag = NULL;
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