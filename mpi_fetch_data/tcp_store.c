/*************************************************************************
  > File Name: tcp_store.cc
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wednesday 10 April 2019 12:26:58 PM IST
 ************************************************************************/
#include "tcp_store.h"
#include "common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int tcp_store_init(tcp_store_t *ts, const char *addr, int port, int backlog, int bucket)
{
    int sockfd = init_tcp_socket(addr, port, backlog);
    if (sockfd == -1)
    {
        return -1;
    }
    uint64_t avg = UINT32_MAX / bucket;

    token_t *tokens = (token_t *)calloc(bucket, sizeof(token_t));
    metric_t *metrics = (metric_t *)calloc(bucket, sizeof(metric_t));
    size_t cache_sz = UINT32_MAX>>5;
    int *cache  = (int *)calloc(cache_sz,szieof(int));
    assert(cache !=NULL);
    assert(tokens != NULL);
    assert(metrics != NULL);

    for (int i = 0; i < bucket; i++)
    {
        tokens[i].rank = i;
        tokens[i].start = i * avg;
        tokens[i].bucket = bucket;
        tokens[i].is_sync = false;
        if (i != (bucket - 1))
        {
            tokens[i].end = (i + 1) * avg - 1;
        }
        else
        {
            tokens[i].end = UINT32_MAX;
        }
        fprintf(stdout, "rank :%d,start=%llu,end=%llu\n", i, tokens[i].start, tokens[i].end);
    }
    ts->sockfd = sockfd;
    ts->tokens = tokens;
    ts->bucket = bucket;
    ts->metrics = metrics;
    ts->cache = cache;
    fprintf(stdout, "======start server on %s:%d=======\n", addr, port);
    return 0;
}
int tcp_store_run(tcp_store_t *ts)
{
    if (ts->sockfd == -1)
    {
        fprintf(stdout, "init socket failed\n");
        return -1;
    }
    fd_set main_set;
    fd_set read_set;
    fd_set write_set;
    FD_ZERO(&main_set);
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(ts->sockfd, &main_set);
    int sfd = ts->sockfd;
    int fdmax = sfd;
    for (;;)
    {
        read_set = main_set;
        write_set = main_set;
        if (select(fdmax + 1, &read_set, NULL, NULL, NULL) == -1)
        {
            close(fdmax);
            break;
        }
        for (int i = sfd; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_set))
            {
                if (i == sfd)
                {
                    struct sockaddr_in cliaddr;
                    int addrlen = sizeof(cliaddr);
                    int clifd = accept(ts->sockfd, (struct sockaddr *)&cliaddr,
                                       (socklen_t *)&addrlen);
                    if (clifd == -1)
                    {
                        fprintf(stdout, "accept new connection failed:%s\n",
                                strerror(errno));
                        break;
                    }

                    FD_SET(clifd, &main_set);
                    fdmax = (fdmax > clifd) ? fdmax : clifd;
                    fprintf(stdout, "accept new connection from %s \n",
                            inet_ntoa(cliaddr.sin_addr));
                }
                else
                {
                    message_t req;
                    memset(&req, 0, sizeof(req));
                    int nbytes = read(i, &req, sizeof(message_t));
                    if (nbytes <= 0)
                    {
                        continue;
                    }
                    switch (req.flag)
                    {
                    case handshake_type:
                        if (!ts->tokens[req.rank].is_sync && req.flag == handshake_type)
                        {

                            if ((nbytes = write(i, &ts->tokens[req.rank], sizeof(token_t))) < 0)
                            {
                                fprintf(stdout, "fd:%d,errno:%d,err:%s\n", i, errno,
                                        strerror(errno));
                                FD_CLR(i, &main_set);
                                continue;
                            }
                            ts->tokens[req.rank].is_sync = true;
                        }
                        break;
                    case read_type:
                        if (ts->tokens[req.rank].is_sync && req.flag == read_type)
                        {
                            uint64_t count = req.data;
                            uint64_t base = ts->tokens[req.rank].end - ts->tokens[req.rank].start;
                            for (uint64_t j = 0; j < count; j++)
                            {
                                message_t msg;
                                msg.rank = req.rank;
                                msg.data = (req.rank == 0) ? (base - (rand() >> 6)) : (base + rand() % base - 1);
                                msg.flag = read_type;
                                int nbytes = write(i, &msg, sizeof(message_t));
                                __sync_fetch_and_add(&ts->metrics[req.rank].count, 1);
                            }
                        }
                        ts->tokens[req.rank].is_sync = false;
                        break;
                    case write_type:
                        if (ts->tokens[req.rank].is_sync && req.flag == write_type)
                        {
                            uint64_t count = rand() % 10;
                            uint64_t value = 0;
                            __sync_fetch_and_add(&ts->metrics[req.rank].count, 1);
                        }
                        ts->tokens[req.rank].is_sync = false;

                        break;
                    case close_type:
                        if (ts->tokens[req.rank].is_sync && req.flag == close_type)
                        {
                            fprintf(stdout, "rank %d ops:%llu   leave!\n", req.rank, 0);
                            FD_CLR(i, &main_set);
                        }
                        break;
                    default:
                        break;
                    }

                    if (ts->metrics[req.rank].count > 0)
                    {
                        fprintf(stdout, "ran [%d] ops:%llu\n", req.rank, ts->metrics[req.rank].count);
                    }
                }
            }
        }
    }
}

void tcp_store_deinit(tcp_store_t *ts)

{

    if (ts->sockfd != -1)
    {
        close(ts->sockfd);
    }
    if (ts->tokens != NULL)
    {
        free(ts->tokens);
    }
    if (ts->metrics != NULL)
    {
        free(ts->metrics);
    }
}
int main(int argc, char *argv[])
{
    tcp_store_t ts;
    tcp_store_init(&ts, "127.0.0.1", atoi(argv[1]), 1024, atoi(argv[2]));
    tcp_store_run(&ts);
    tcp_store_deinit(&ts);
    return 0;
}
