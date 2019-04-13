/*************************************************************************
  > File Name: token_ring.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sat 13 Apr 2019 10:56:42 AM CST
 ************************************************************************/

#ifndef _TOKEN_RING_H
#define _TOKEN_RING_H
#include <stdint.h>
struct tokenInfo
{
  int rank;
  uint64_t start;
  uint64_t end;
};
struct handShakeInfo {
  
};
#endif
