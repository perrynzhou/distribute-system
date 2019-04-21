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
#include <pthread.h>
typedef struct {
   int sockfd;
   int  rank;
   int  rank_size;
   int  type;
   uint64_t op_count;
   token_t  token;
   metric_t *metrics;
   mpi_thread_t  *threads;
}mpi_processor_t;
int mpi_processor_init(mpi_processor_t *mp,const char *addr,int port,int rank,int rank_size,int type);
void mpi_processor_run(mpi_processor_t *mp);
void mpi__processor_deinit(mpi_processor_t *mp);
#endif
