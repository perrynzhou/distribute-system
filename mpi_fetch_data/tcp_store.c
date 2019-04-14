/*************************************************************************
  > File Name: tcp_server.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wednesday 10 April 2019 12:26:58 PM IST
 ************************************************************************/
#include "tcp_store.h"
#include "common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define TCP_ADDR "127.0.0.1"
int tcp_store_init(struct tcp_store *ts, int port, int backlog, int bucket) {
  ts->sockfd = init_tcp_socket(TCP_ADDR, port, backlog);
  if (ts->sockfd == -1) {
    return -1;
  }
  ts->port = port;
  uint64_t avg = UINT32_MAX / bucket;
  ts->tokens = (struct token *)calloc(sizeof(struct token), bucket);
  for (int i = 0; i < bucket; i++) {
    ts->tokens[i].rank = i;
    ts->tokens[i].start = i * avg;
    ts->tokens[i].is_sync = false;
    if (i != (bucket - 1)) {
      ts->tokens[i].end = (i + 1) * avg - 1;
    } else {
      ts->tokens[i].end = UINT32_MAX;
    }
    fprintf(stdout, "rank :%d,start=%llu,end=%llu\n", i, ts->tokens[i].start,
            ts->tokens[i].end);
  }
  return 0;
}
void tcp_store_run(struct tcp_store *ts) {
  if (ts->sockfd == -1) {
    fprintf(stdout, "init socket failed\n");
    return;
  }
  fprintf(stdout, "======start server at %d=======\n", ts->port);
  fd_set main_set;
  fd_set read_set;
  fd_set write_set;
  FD_ZERO(&main_set);
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(ts->sockfd, &main_set);
  int sfd = ts->sockfd;
  int fdmax = sfd;

  for (;;) {
    read_set = main_set;
    write_set = main_set;
    if (select(fdmax + 1, &read_set, &write_set, NULL, NULL) == -1) {
      close(fdmax);
      break;
    }
    for (int i = sfd; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_set)) {
        if (i == sfd) {
          struct sockaddr_in cliaddr;
          int addrlen = sizeof(cliaddr);
          int clifd = accept(ts->sockfd, (struct sockaddr *)&cliaddr,
                             (socklen_t *)&addrlen);
          if (clifd == -1) {
            fprintf(stdout, "accept new connection failed:%s\n",
                    strerror(errno));
            continue;
          }

          FD_SET(clifd, &main_set);
          fdmax = (fdmax < clifd) ? clifd : fdmax;
          fprintf(stdout, "accept new connection from %s \n",
                  inet_ntoa(cliaddr.sin_addr));
        } else {
          struct request req;
          memset(&req, 0, sizeof(req));
          int nbytes = read(i, &req, sizeof(struct request));
          if (nbytes < 0) {
            perror("read");
            FD_CLR(i, &main_set);
            continue;
          }
          switch (req.flag) {
          case handshake_type:
            if (!ts->tokens[req.rank].is_sync && req.flag == handshake_type) {
              if ((nbytes = write(i, &ts->tokens[req.rank],
                                  sizeof(struct token))) < 0) {
                fprintf(stdout, "fd:%d,errno:%d,err:%s\n", i, errno,
                        strerror(errno));
                FD_CLR(i, &main_set);
                continue;
              }
              ts->tokens[req.rank].is_sync = true;
            }
            break;
          case read_type:
            break;
          case write_type:
            break;
          case close_type:
            break;
          }
        }
      }
    }
  }
}

void tcp_store_deinit(struct tcp_store *ts) {

  if (ts->sockfd != -1) {
    close(ts->sockfd);
  }
  if (ts->store != NULL) {
    free(ts->store);
  }
}
int main(int argc, char *argv[]) {
  struct tcp_store ts;
  sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
  tcp_store_init(&ts, atoi(argv[1]), 1024, atoi(argv[2]));
  tcp_store_run(&ts);
  tcp_store_deinit(&ts);
  return 0;
}
