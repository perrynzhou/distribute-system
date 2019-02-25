/*************************************************************************
  > File Name: util.cc
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/

#include "util.h"
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
int initSocket(int port, int backlog)
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in srvaddr;
  if (sock == -1)
  {
    return -1;
  }

  memset(&srvaddr, 0, sizeof(srvaddr));
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  srvaddr.sin_port = htons(port);

  if (bind(sock, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
  {
    return -1;
  }
  if (listen(sock, backlog) == -1)
  {
    return -1;
  }
  return sock;
}
void initRandomString(int size, char *str)
{
  size_t len = strlen(fill) - 1;
  for (int i = 0; i < size; i++)
  {
    str[i] = fill[rand() % len];
  }
}