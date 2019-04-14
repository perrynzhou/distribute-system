/*************************************************************************
  > File Name: mpi_processor.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Fri 12 Apr 2019 05:10:24 PM CST
 ************************************************************************/
#include "mpi_processor.h"
#include <arpa/inet.h>
#include <errno.h>
#include <mpi.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
int mpi_processor_init(struct mpi_processor *mp, const char *remote_host,
                       int port, int rank) {
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(remote_host);

  mp->token.rank = rank;

  fprintf(stdout, "rank %d start connect %s:%d\n", mp->token.rank, remote_host,
          port);
  if (connect(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("connect");
    return -1;
  }
  struct request req;
  req.rank = rank;
  req.data = -1;
  req.flag = handshake_type;
  if (write(sfd, &req, sizeof(struct request)) <0) {
    close(sfd);
    fprintf(stdout, "sync  to tcp_store failed:%s\n", strerror(errno));
    return -1;
  }
  
  memset(&mp->token, 0, sizeof(struct token));
  if (read(sfd, &mp->token, sizeof(struct token)) <0) {
     close(sfd);
    fprintf(stdout, "sync token from tcp_store failed\n");
    return -1;
  }
  //fprintf(stdout, "======mpi rank:%d,start:%llu,end:%llu ========\n", mp->token.rank,mp->token.start, mp->token.end);
  char buf[128] = {'\0'};
  int avg = ((mp->token.end - mp->token.start) >> 5);
  size_t sz = (avg % 32 == 0) ? avg : avg + 1;
  if ((mp->token.rank%2) == 0) {
    mp->type = 0; // is reader
    mp->store = NULL;
    sprintf((char *)&buf, "%s", "writer");
  } else {
    mp->store = (uint32_t *)calloc(sizeof(uint32_t), sz);
    mp->type = 1; // is writer
    sprintf((char *)&buf, "%s", "reader");
  }
  mp->serverfd = sfd;
  fprintf(stdout, "mpi rank:%d,token.start=%ld,token.end:%ld,type:%s\n",
          mp->token.rank, mp->token.start, mp->token.end, buf);
  return 0;
}
void mpi_processor_run(struct mpi_processor *mp) {
  if (mp->type == 0) {
  } else {
  }
}
void mpi_processor_deinit(struct mpi_processor *p) {
  sleep(5);
  if (p->serverfd != -1) {
    close(p->serverfd);
  }
  if (p->store != NULL) {
    free(p->store);
  }
}
int main(int argc, char *argv[]) {
  int rank, size;
  int token;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  signal(SIGPIPE, SIG_IGN);

  struct mpi_processor mp;
  const char *remote_host = argv[1];
  int port = atoi(argv[2]);
  mpi_processor_init(&mp, remote_host, port, rank);
  mpi_processor_deinit(&mp);
  MPI_Finalize();
}
