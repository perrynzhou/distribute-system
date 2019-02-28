/*************************************************************************
  > File Name: topk.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四  2/28 13:45:36 2019
 ************************************************************************/

#ifndef _TOPK_H
#define _TOPK_H
typedef int (*objectCmp)(void *a,void *b);
typedef void (*objectSwap)(void *a,void *b);
int objectQuickObject(void *arr,int esize,int l,int r,int k,objectCmp cmp,objectSwap swap);
int findKthLargest(void *arr,int l,int r,int k);
#endif
