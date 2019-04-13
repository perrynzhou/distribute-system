/*************************************************************************
  > File Name: tcp_server.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Wednesday 10 April 2019 12:26:58 PM IST
 ************************************************************************/
#include "tcp_server.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

int tcpServerInit(struct tcpServer *ts, int port, int backlog, int bucket)
{

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        fprintf(stdout, "socket:%s\n", strerror(errno));
        goto initErr;
    }
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        fprintf(stdout, "setsockopt:%s\n", strerror(errno));
        goto initErr;
    }
    struct sockaddr_in srv;
    memset(srv.sin_zero, '\0', sizeof(s->sin_zero));

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(addr.c_str());
    srv.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr *)&srv, sizeof(srv)) == -1)
    {
        fprintf(stdout, "bind:%s\n", strerror(errno));
        goto initErr;
    }
    if (listen(sockfd, backlog) == -1)
    {
        fprintf(stdout, "listen:%s\n", strerror(errno));
        goto initErr;
    }
    ts->sockfd = sockfd;
    ts->port = port;
    uint64_t avg = UINT32_MAX / bucket;
    ts->tokens = (struct tokenInfo *)calloc(sizeof(struct tokenInfo), bucket);
    for (int i = 0; i < bucket; i++)
    {
        ts->tokens[i].rank = i;
        ts->tokens[i].start = i * avg;
        fprintf(stdout, "rank :%ld,start=%ld,end=%ld\n", i, ts->tokens[i].start, ts->tokens[i].end);
        if (i != (bucket - 1))
        {
            ts->tokens[i].end = (i + 1) * avg - 1;
            break;
        }
        ts->tokens[i].end = UINT32_MAX;
        fprintf(stdout, "rank :%ld,start=%ld,end=%ld\n", i, ts->tokens[i].start, ts->tokens[i].end);
    }
    return 0;
initErr:
    if (sockfd != -1)
    {
        close(sockfd);
    }
    return -1;
}
void tcpServerRun(struct tcpServer *ts)
{
    if (ts->sockfd == -1)
    {
        fprintf(stdout, "init socket failed\n");
        return;
    }
    fprintf(stdout, "start server at %d\n", ts->port);
    fd_set main_set;
    fd_set read_set;
    FD_ZERO(&main_set);
    FD_ZERO(&read_set);
    FD_SET(sockfd_, &main_set);
    int sfd = sockfd_;
    int fdmax = sfd;
    struct sockaddr_in cliaddr;
    for (;;)
    {
        read_set = main_set;
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
                    int addrlen = sizeof(cliaddr);
                    int clifd = accept(ts->sockfd, (struct sockaddr *)&cliaddr, &addrlen);
                    if (clifd == -1)
                    {
                        fprintf(stdout, "accept new connection failed:%s\n", strerror(errno));
                        continue;
                    }
                    FD_SET(clifd, &main_set);
                    fdmax = (fdmax < clifd) ? clifd : fdmax;
                    fprintf(stdout, "accept new connection from %s \n", inet_ntoa(cliaddr.sin_addr));
                }
                else
                {
                    char buf[64];
                    int nbytes = recv(i, buf, sizeof(buf), 0);
                    if (nbytes < 0)
                    {
                        continue;
                    }
                    size_t len = strlen(buf);
                    if (len == 0)
                    {
                        FD_CLR(i, &main_set);
                    }
                }
            }
            else
            {
                int j = 0;
                for (j = 0; j < fdmax; j++)
                {
                    if (FD_ISSET(j, &main_set))
                    {
                        if (j != ts->sockfd && j != i)
                        {
                            char buf[32];
                            send(j, buf, 32, 0);
                        }
                    }
                }
            }
        }
    }
}
}
void tcpServerDeinit(struct tcpServer *ts)()
{

    if (ts->sockfd != NULL)
    {
        close(ts->sockfd)
    }
}
#ifdef TEST
int main(void)
{
    TcpServer ts("127.0.0.1", 8787, 1024, 5);
    ts.Run();
    return 0;
}
#endif
