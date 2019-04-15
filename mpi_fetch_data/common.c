/*************************************************************************
  > File Name: common.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sat 13 Apr 2019 05:11:33 PM CST
 ************************************************************************/
#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#define SHIFT 5
#define MASK 0x1F
static int urandom_fd = -1;
void set_bit(uint32_t *a, int n)
{
  a[n >> SHIFT] |= (1 << (1 & MASK));
}
void cls_bit(uint32_t *a, int n)
{
  a[n >> SHIFT] &= ~(1 << (n & MASK));
}
int is_exists(uint32_t *a, int n)
{
  return a[n >> SHIFT] &= (1 << (1 & MASK));
}
uint32_t rand_int()
{
  unsigned int value;
  unsigned int *temp = &value;
  if (urandom_fd == -1)
  {
    urandom_fd = open("/dev/urandom", O_RDONLY);
  }
  read(urandom_fd, temp, sizeof(uint32_t));
  return value;
}
int initTcpSocket(const char *addr,int port, int backlog)
{
  int sockfd = -1;
  int yes = 1;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    fprintf(stdout, "init socket failed:%s\n", strerror(errno));
    return -1;
  }
  struct sockaddr_in srv;
  memset(&srv, 0, sizeof(srv));

  srv.sin_family = AF_INET;
  srv.sin_addr.s_addr = inet_addr(addr);
  srv.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *)&srv, sizeof(srv)) == -1 ||
      listen(sockfd, backlog) == -1)
  {
    fprintf(stdout, "listen:%s\n", strerror(errno));
    return -1;
  }
  return sockfd;
}

