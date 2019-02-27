/*************************************************************************
  > File Name: util.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:24:18 2019
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define ASSERT(_x) 
#define NOT_REACHED()
#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
typedef struct UdpInfo  {
  struct sockaddr_in addr;
  int sock;
}UdpInfo;
void stacktraceFd(int fd);
int strToi(uint8_t *line, size_t n);
int initTcpSocket(const char *addr,int port, int backlog);
int initUdpSocket(const char *addr,int port,UdpInfo *ui);
void randomString(int size, char *str);
int checkHeartbeat(UdpInfo *info, void *pkg, int pkg_len);
#endif
