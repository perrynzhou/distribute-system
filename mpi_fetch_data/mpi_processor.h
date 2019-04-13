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
struct mpiProcessor {
    int serverfd;
    int type; // 0 is reader,1 is writer,2 is producer
    struct tokenInfo token;
    uint8_t *store;
};
int mpiProcessorInit(char *remote_host,int port,int type);
void mpiProcessorDeinit(struct mpiProcessor *p);
#endif
