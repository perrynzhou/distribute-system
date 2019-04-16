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
#include <cstdint>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TcpStore::TcpStore(const char* addr, int port, int bucket)
    : addr_(addr)
    , port_(port)
    , sockfd_(-1)
    , backlog_(1024)
    , bucket_(bucket)
    , tokens_(nullptr)
{
}

int TcpStore::Prepare()
{
    sockfd_ = initTcpSocket(addr_.c_str(), port_, backlog_);
    uint64_t avg = UINT32_MAX / bucket_;

    tokens_ = new Token[bucket_];
    countor_ = new uint64_t[bucket_];
    if (tokens_ != nullptr) {
        for (int i = 0; i < bucket_; i++) {
            countor_[i] = 0;
            tokens_[i].rank_ = i;
            tokens_[i].start_ = i * avg;
            tokens_[i].bucket_ = bucket_;
            tokens_[i].is_sync_ = false;
            if (i != (bucket_ - 1)) {
                tokens_[i].end_ = (i + 1) * avg - 1;
            } else {
                tokens_[i].end_ = UINT32_MAX;
            }
            fprintf(stdout, "rank :%d,start=%llu,end=%llu\n", i, tokens_[i].start_,
                    tokens_[i].end_);
        }
        return 0;
    }
    return -1;
}
void TcpStore::Run()
{
    if (Prepare() == -1) {
        fprintf(stdout, "init socket failed\n");
        return;
    }
    fprintf(stdout, "======start server on %s:%d=======\n", addr_.c_str(), port_);
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
                    int clifd = accept(sockfd_, (struct sockaddr*)&cliaddr,
                                       (socklen_t*)&addrlen);
                    if (clifd == -1) {
                        fprintf(stdout, "accept new connection failed:%s\n",
                                strerror(errno));
                        break;
                    }

                    FD_SET(clifd, &main_set);
                    fdmax = (fdmax > clifd) ? fdmax : clifd;
                    fprintf(stdout, "accept new connection from %s \n",
                            inet_ntoa(cliaddr.sin_addr));
                } else {
                    Message req;
                    memset(&req, 0, sizeof(req));
                    int nbytes = read(i, &req, sizeof(Message));
                    if (nbytes <= 0) {
                        continue;
                    }
                    switch (req.flag_) {
                    case HandshakeType:
                        if (!tokens_[req.rank_].is_sync_ && req.flag_ == HandshakeType) {

                            if ((nbytes = write(i, &tokens_[req.rank_], sizeof(Token))) < 0) {
                                fprintf(stdout, "fd:%d,errno:%d,err:%s\n", i, errno,
                                        strerror(errno));
                                FD_CLR(i, &main_set);
                                continue;
                            }
                            tokens_[req.rank_].is_sync_ = true;
                        }
                        break;
                    case ReadType:
                        if (tokens_[req.rank_].is_sync_ && req.flag_ == ReadType) {
                            uint64_t count = req.data_;
                            uint64_t base = tokens_[req.rank_].end_ - tokens_[req.rank_].start_;
                            for (uint64_t j = 0; j < count; j++) {
                                Message msg;
                                msg.rank_ = req.rank_;
                                msg.data_ = (req.rank_ == 0) ? (base - (rand() >> 6)) : (base + rand() % base - 1);
                                msg.flag_ = ReadType;
                                int nbytes = write(i, &msg, sizeof(Message));
                                __sync_fetch_and_add(&countor_[req.rank_], 1);
                            }
                        }
                        tokens_[req.rank_].is_sync_ = false;
                        break;
                    case WriteType:
                        if (tokens_[req.rank_].is_sync_ && req.flag_ == WriteType) {
                            uint64_t count = rand() % 10;
                            uint64_t value = 0;
                            __sync_fetch_and_add(&countor_[req.rank_], 1);
                        }
                        tokens_[req.rank_].is_sync_ = false;

                        break;
                    case CloseType:
                        if (tokens_[req.rank_].is_sync_ && req.flag_ == CloseType) {
                            fprintf(stdout, "rank %d ops:%llu   leave!\n", req.rank_, countor_[req.rank_]);
                            FD_CLR(i, &main_set);
                        }
                        break;
                    default:
                        break;
                    }
                    if (countor_[req.rank_] > 0) {
                        fprintf(stdout, "rank %d op:%d\n", req.rank_, countor_[req.rank_]);
                    }
                }
            }
        }
    }
}

TcpStore::~TcpStore()
{

    if (sockfd_ != -1) {
        close(sockfd_);
    }
}
int main(int argc, char* argv[])
{
    TcpStore ts("127.0.0.1", atoi(argv[1]), atoi(argv[2]));
    ts.Run();
    return 0;
}
