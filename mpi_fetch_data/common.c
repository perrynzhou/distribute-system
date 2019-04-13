/*************************************************************************
  > File Name: common.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sat 13 Apr 2019 05:11:33 PM CST
 ************************************************************************/
#include "common.h"
#include <stdio.h>
void setBit(uint32_t *a, int n)
{
  a[n >> 32] |= (1 << (1 & 0x1F));
}
void clsBit(uint32_t *a, int n)
{
  a[n >> 32] &= ~(1 << (n & 0x1F));
}
int isExists(uint8_t *a, int n)
{
  return a[n >> 32] &= (1 << (1 & 0x1F));
}
uint32_t randInt() {
   srand(time(0)+rand()%16);
   return rand();
}
