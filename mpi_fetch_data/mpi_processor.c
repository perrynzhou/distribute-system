/*************************************************************************
  > File Name: mpi_processor.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Fri 12 Apr 2019 05:10:24 PM CST
 ************************************************************************/
#include "mpi_processor.h"

#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <mpi.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int mpi_processor_init(mpi_processor_t *mp, const char *addr, int port, int rank, int rank_size, int op_count)
{
    int sfd = init_tcp_client(addr, port);
    if (sfd == -1)
    {
        return -1;
    }

    message_t req;
    req.rank = rank;
    req.data = -1;
    req.flag = handshake_type;
    if (write(sfd, &req, sizeof(message_t)) < 0)
    {
        close(sfd);
        fprintf(stdout, "sync  to tcp_store failed:%s\n", strerror(errno));
        return -1;
    }

    if (read(sfd, &mp->token, sizeof(token_t)) < 0)
    {
        close(sfd);
        fprintf(stdout, "sync token from tcp_store failed\n");
        return -1;
    }
    fprintf(stdout, "rank %d start connect %s:%d\n", mp->token.rank, addr,
            port);
    mp->token.rank = rank;
    mp->rank_size = rank_size;
    mp->rank = rank;
    mp->op_count = op_count;

    char buf[128] = {'\0'};
    int avg = ((mp->token.end - mp->token.start) >> 5);
    size_t sz = (avg % 32 == 0) ? avg : avg + 1;
    if (rank < mp->token.bucket)
    {
        mp->type = read_type; // is reader
        sprintf((char *)&buf, "%s", "writer");
        mp->thd = NULL;
    }
    else
    {
        mp->type = write_type; // is writer
        sprintf((char *)&buf, "%s", "reader");
        mp->thd = (pthread_t *)calloc(sizeof(pthread_t), ts->bucket);
        assert(mp->thd != NULL);
    }
    mp->metrics = (metric_t *)calloc(sizeof(metric_t), rank_size);
    for (int i = 0; i < mp->rank_size; i++)
    {
        mp->metrics[i].rank = (i == rank) ? mp->rank : -1;
        mp->metrics[i].count = 0;
    }
    mp->sockfd = sfd;
    fprintf(stdout, "[handeshake]:mpi rank:%d,rank size:%d,token.start=%ld,token.end:%ld,bucket:%d,type:%s\n",
            mp->token.rank, rank_size, mp->token.start, mp->token.end, mp->token.bucket, buf);
    return 0;
}
void mpi_processor_run(mpi_processor_t *mp)
{
    if (mp->sockfd == -1)
    {
        fprintf(stdout, "MpiProcessor Prepare Failed\n");
        return;
    }

    // pass data count to server
    message_t req;
    memset(&req, 0, sizeof(req));
    req.rank = mp->rank;
    req.flag = mp->type;
    req.data = mp->op_count;
    uint64_t base = mp->token.end - mp->token.start;
    write(mp->sockfd, &req, sizeof(req));
    uint64_t count = req.data;
    for (uint64_t i = 0; i < count; i++)
    {
        message_t msg;
        msg.rank = mp->rank;
        memset(&msg, 0, sizeof(message_t));
        int nbytes = -1;
        if (mp->rank < mp->token.bucket)
        {
            msg.flag = read_type;
            nbytes = read(mp->sockfd, &msg, sizeof(message_t));
            fprintf(stdout, "## rank[%d] msg.rank=%d,msg.data=%llu,msg.falg=%d\n", mp->rank, msg.rank, msg.data, msg.flag);
        }
        else
        {
            msg.flag = write_type;
            msg.data = UINT32_MAX % rand();
            nbytes = write(mp->sockfd, &msg, sizeof(message_t));
        }
    }
}
void mpi__processor_deinit(mpi_processor_t *mp)
{
    if (mp->sockfd != -1)
    {
        close(mp->sockfd);
    }
    if (mp->metrics != NULL)
    {
        free(mp->metrics);
    }
    if (mp->thd != NULL)
    {
        free(mp->thd);
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

    const char *addr = argv[1];
    int port = atoi(argv[2]);
    int count = atoi(argv[3]);
    mpi_processor_t mp;
    mpi_processor_init(&mp, addr, port, rank, size, count);
    mpi_processor_run(&mp);
    mpi__processor_deinit(&mp);
    MPI_Finalize();
}
