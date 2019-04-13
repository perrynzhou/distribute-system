/*************************************************************************
  > File Name: token_ring.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sat 13 Apr 2019 10:56:42 AM CST
 ************************************************************************/

#ifndef _TOKEN_RING_H
#define _TOKEN_RING_H
#include <stdint.h>
#include <stdbool.h>
struct tokenInfo
{
  int rank;
  uint64_t start;
  uint64_t end;
};
struct requestBody
{
  int rank;
  bool is_sync;
};

void setBit(uint32_t *a, int n);
void clsBit(uint32_t a, int n);
bool isExists(uint32_t *a, int n);
uint32_t randInt();
#endif
