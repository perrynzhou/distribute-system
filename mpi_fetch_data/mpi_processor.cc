/*************************************************************************
  > File Name: mpi_processor.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Fri 12 Apr 2019 05:10:24 PM CST
 ************************************************************************/
#include "mpi_processor.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <mpi.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

MpiProcessor::MpiProcessor(const char* remote_host, int port, int rank, int rank_size, int op_count)
    : addr_(remote_host)
    , port_(port)
    , serverfd_(-1)
    , rank_(rank)
    , rank_size_(rank_size)
    , op_count_(op_count)
{
}
int MpiProcessor::Prepare()
{
    int sfd = initTcpClient(addr_.c_str(), port_);
    if (sfd == -1) {
        return -1;
    }
    token_.rank_ = rank_;
    fprintf(stdout, "rank %d start connect %s:%d\n", token_.rank_, addr_.c_str(),
            port_);
    Message req;
    req.rank_ = rank_;
    req.data_ = -1;
    req.flag_ = HandshakeType;
    if (write(sfd, &req, sizeof(Message)) < 0) {
        close(sfd);
        fprintf(stdout, "sync  to tcp_store failed:%s\n", strerror(errno));
        return -1;
    }

    if (read(sfd, &token_, sizeof(Token)) < 0) {
        close(sfd);
        fprintf(stdout, "sync token from tcp_store failed\n");
        return -1;
    }
    char buf[128] = { '\0' };
    int avg = ((token_.end_ - token_.start_) >> 5);
    size_t sz = (avg % 32 == 0) ? avg : avg + 1;
    if (rank_ < token_.bucket_) {
        type_ = ReadType; // is reader
        sprintf((char*)&buf, "%s", "writer");
    } else {
        type_ = WriteType; // is writer
        sprintf((char*)&buf, "%s", "reader");
    }
    serverfd_ = sfd;
    metrics_ = new Metric[rank_size_];
    for (int i = 0; i < rank_size_; i++) {
        metrics_[i].type_ = (i == rank_) ? type_ : -1;
        metrics_[i].count_ = 0;
    }
    fprintf(stdout, "[handeshake]:mpi rank:%d,rank size:%d,token.start=%ld,token.end:%ld,bucket:%d,type:%s\n",
            token_.rank_, rank_size_, token_.start_, token_.end_, token_.bucket_, buf);
    return 0;
}
void MpiProcessor::Run()
{
    if (Prepare() != 0) {
        fprintf(stdout, "MpiProcessor Prepare Failed\n");
        return;
    }

    // pass data count to server
    Message req;
    memset(&req, 0, sizeof(req));
    req.rank_ = rank_;
    req.flag_ = type_;
    req.data_ = op_count_;
    uint64_t base = token_.end_ - token_.start_;
    write(serverfd_, &req, sizeof(req));
    uint64_t count = req.data_;
    for (uint64_t i = 0; i < count; i++) {
        Message msg;
        msg.rank_ = rank_;
        memset(&msg, 0, sizeof(Message));
        int nbytes = -1;
        if (rank_ < token_.bucket_) {
            msg.flag_ = ReadType;
            nbytes = read(serverfd_, &msg, sizeof(Message));
            fprintf(stdout, "## rank[%d] msg.rank=%d,msg.data=%llu,msg.falg=%d\n", rank_, msg.rank_, msg.data_, msg.flag_);
        } else {
            msg.flag_ = WriteType;
            msg.data_ = base + rand() % base;
            nbytes = write(serverfd_, &msg, sizeof(Message));
        }
    }
}
MpiProcessor::~MpiProcessor()
{
    if (serverfd_ != -1) {
        close(serverfd_);
    }
    if (metrics_ != nullptr) {
        delete[] metrics_;
    }
}
int main(int argc, char* argv[])
{
    int rank, size;
    int token;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char* host = argv[1];
    int port = atoi(argv[2]);
    int count = atoi(argv[3]);
    MpiProcessor worker(host, port, rank, size, count);
    worker.Run();
    MPI_Finalize();
}
