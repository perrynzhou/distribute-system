/*************************************************************************
  > File Name: tcp_store.cc
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wednesday 10 April 2019 12:26:58 PM IST
 ************************************************************************/
#include "tcp_store.h"
#include "common.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
TcpStore::TcpStore(const char *addr, int port, int bucket)
    : port_(port), bucket_(bucket) {
  sockfd_ = initTcpSocket(addr, port, backlog_);
  uint64_t avg = UINT32_MAX / bucket;
  tokens_ = new Token[bucket];
  for (int i = 0; i < bucket; i++) {
    tokens_[i].rank = i;
    tokens_[i].start = i * avg;
    tokens_[i].is_sync = false;
    if (i != (bucket - 1)) {
      tokens_[i].end = (i + 1) * avg - 1;
    } else {
      tokens_[i].end = UINT32_MAX;
    }
    fprintf(stdout, "rank :%d,start=%llu,end=%llu\n", i, tokens_[i].start,
            tokens_[i].end);
  }
}
void TcpStore::Run() {
  if (sockfd_ == -1) {
    fprintf(stdout, "init socket failed\n");
    return;
  }
  fprintf(stdout, "======start server at %d=======\n", port_);
  fd_set main_set;
  fd_set read_set;
  fd_set write_set;
  FD_ZERO(&main_set);
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(sockfd_, &main_set);
  int sfd = sockfd_;
  int fdmax = sfd;

  for (;;) {
    read_set = main_set;
    write_set = main_set;
    if (select(fdmax + 1, &read_set, &write_set, nullptr, nullptr) == -1) {
      close(fdmax);
      break;
    }
    for (int i = sfd; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_set)) {
        if (i == sfd) {
          struct sockaddr_in cliaddr;
          int addrlen = sizeof(cliaddr);
          int clifd = accept(sockfd_, (struct sockaddr *)&cliaddr,
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
          struct Request req;
          memset(&req, 0, sizeof(req));
          int nbytes = read(i, &req, sizeof(Request));
          if (nbytes < 0) {
            perror("read");
            FD_CLR(i, &main_set);
            continue;
          }
          switch (req.flag) {
          case handshake_type:
            if (!tokens_[req.rank].is_sync && req.flag == handshake_type) {
              if ((nbytes = write(i, &tokens_[req.rank], sizeof(Token))) < 0) {
                fprintf(stdout, "fd:%d,errno:%d,err:%s\n", i, errno,
                        strerror(errno));
                FD_CLR(i, &main_set);
                continue;
              }
              tokens_[req.rank].is_sync = true;
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

TcpStore::~TcpStore() {

  if (sockfd_ != -1) {
    close(sockfd_);
  }
  if (store_ != NULL) {
    delete[] store_;
  }
}
int main(int argc, char *argv[]) {
  TcpStore ts("127.0.0.1", atoi(argv[1]), atoi(argv[2]));
  ts.Run();
  return 0;
}
