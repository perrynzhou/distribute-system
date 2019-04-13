/*************************************************************************
  > File Name: mpi_processor.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Fri 12 Apr 2019 05:10:24 PM CST
 ************************************************************************/
#include "mpi_processor.h"
#include <mpi.h>
#include <stdio.h>
int mpiProcessorInit(char *remote_host, int port, int type)
{
  int cfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);                
  servaddr.sin_addr.s_addr = inet_addr(remote_host); 

  if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("connect");
    return -1;
  }
  return 0;
}
