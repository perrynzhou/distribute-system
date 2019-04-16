/*************************************************************************
  > File Name: mpi_processor.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Fri 12 Apr 2019 05:10:19 PM CST
 ************************************************************************/

#ifndef _MPI_PROCESSOR_H
#define _MPI_PROCESSOR_H
#include "common.h"
#include <string>
#include <cstdint>
using namespace std;
class MpiProcessor
{
  string addr_;
  int port_;
  int serverfd_;
  int rank_;
  int rank_size_;
  int type_;
  int op_count_;
  Metric *metrics_;
  int Prepare();

public:
  MpiProcessor(const char *remote_host, int port, int rank, int rank_size,int op_count);
  void Run();
  ~MpiProcessor();
  Token token_;
};
#endif
