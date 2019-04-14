/*************************************************************************
  > File Name: mpi_processor.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Fri 12 Apr 2019 05:10:19 PM CST
 ************************************************************************/

#ifndef _MPI_PROCESSOR_H
#define _MPI_PROCESSOR_H
#include "common.h"
#include <stdint.h>

struct mpi_processor {
    int serverfd;
    uint8_t type; // 0 is handshake,1 is read,2 is wirite,3 is close;
    struct token token;
    uint32_t *store;
    int  rank_size;
    struct metric *mcs;
};
int mpi_processor_init(struct mpi_processor *mp,const char *remote_host,int port,int rank,int rank_size);
void mpi_processor_run(struct mpi_processor *mp);
void mpi_processor_deinit(struct mpi_processor *p);
#endif
