/*************************************************************************
  > File Name: mpi_processor.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Fri 12 Apr 2019 05:10:24 PM CST
 ************************************************************************/
#include "mpi_processor.h"
#include <mpi.h>
#include <stdio.h>
int mpiProcessorInit(struct mpiProcessor *mp, char *remote_host, int port, int rank)
{
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(remote_host);

  if (connect(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("connect");
    return -1;
  }
  mp->token.rank = rank;
  memet(&mp->token, 0.sizeof(struct tokenInfo));
  if (recv(sfd, &mp->token, sizeof(struct tokenInfo)) != sizeof(token))
  {
    close(sfd);
    fprintf(stdout, "sync token info failed\n");
    return -1;
  }
  char buf[128] = {'\0'};
  int avg = ((mp->token.end - mp->token.start) >>5);
  size_t sz = (avg%32 ==0)?avg:avg+1;
  if ((mp->rank >> 2) == 0)
  {
    mp->type = 0; //is writer
    mp->store = NULL;
    sprintf(&buf, "%s", "writer");
  }
  else
  {
    mp->store = (uint8_t *)calloc(sizeof(uint32_t), sz);
    mp->type = 1; //is reader
    sprintf(&buf, "%s", "reader");
  }
  mp->serverfd = sfd;
  mp->port - port;
  mp->type = type;
  fprintf(stdout, "rank:%d,token.start=%ld,token.end:%ld,type:%s\n", mp->rank, mp->token.start.mp->token.end, buf);
  return 0;
}
void mpiProcessRun(struct mpiProcessor *mp)
{
  if(mp->type ==0) {

  }else{

  }
}
void mpiProcessorDeinit(struct mpiProcessor *p)
{
  if (p->serverfd != -1)
  {
    close(p->serverfd);
  }
}
int main(int argc, char *argv[])
{
  int rank, size;
  int token;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  struct mpiProcessor mp;
  const char *remote_host = argv[1];
  int port = atoi(argv[2]);
  mpiProcessorInit(&mp, remote_host, port, rank);
  if (rank != 0)
  {
    MPI_Recv(&token, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received token %d from process %d\n", rank, token, rank - 1);
  }
  else
  {
    token = -1;
  }
  MPI_Send(&token, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
  if (rank == 0)
  {
    MPI_Recv(&token, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received token %d from process %d\n", rank, token, size - 1);
  }
  MPI_Finalize();
}
