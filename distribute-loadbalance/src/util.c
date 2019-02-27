/*************************************************************************
  > File Name: util.cc
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/

#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
const char *normString = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
void stacktraceFd(int fd)
{
#ifdef NC_HAVE_BACKTRACE
  void *stack[64];
  int size;

  size = backtrace(stack, 64);
  backtrace_symbols_fd(stack, size, fd);
#endif
}
int strToi(uint8_t *line, size_t n)
{
  int value;

  if (n == 0)
  {
    return -1;
  }

  for (value = 0; n--; line++)
  {
    if (*line < '0' || *line > '9')
    {
      return -1;
    }

    value = value * 10 + (*line - '0');
  }

  if (value < 0)
  {
    return -1;
  }

  return value;
}
int initTcpSocket(int port, int backlog)
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
int initUdpSocket(int port)
{
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
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
  return sock;
}
void initRandomString(int size, char *str)
{
  size_t len = strlen(normString) - 1;
  for (int i = 0; i < size; i++)
  {
    str[i] = normString[rand() % len];
  }
}