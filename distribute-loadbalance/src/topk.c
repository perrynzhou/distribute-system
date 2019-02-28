/*************************************************************************
  > File Name: topk.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 四  2/28 13:45:43 2019
 ************************************************************************/
#include "topk.h"
#include <stdio.h>
int objectQuickObject(void *arr, int esize, int l, int r, int k, objectCmp cmp, objectSwap swap)
{
  if (NULL == arr || NULL == cmp || l < r)
  {
    return -1;
  }
  int i = l, j = r;
  void *base = (char *)arr + l * esize;
  if (l < r)
  {
    while (i < j)
    {
      void *basej = (char *)&arr + (j * esize);
      void *basei = (char *)&arr + (i * esize);
      while (i < j && cmp(base, basej) > 0)
      { //base>basej
        j--;
      }
      if (i < j)
      {
        basej = (char *)&arr + (j * esize);
        swap(basei, basej);
        i++;
      }
      while (i < j && cmp(base, basei) <= 0)
      { //base <=basei
        i++;
      }
      if (i < j)
      {
        basej = (char *)&arr + (j * esize);
        swap(basej, basei);
        j--;
      }
      basei = (char *)&arr + (i * esize);
      swap(basei, base);
    }
    if (k < 0)
    {
      objectQuickObject(arr, esize, l, i - 1, k, cmp, swap);
      objectQuickObject(arr, esize, i + 1, r, k, cmp, swap);
    }
    else
    {
      if ((j - l + 1) > k)
      {
        objectQuickObject(arr, esize, l, i - 1, k, cmp, swap);
      }
      else
      {
        objectQuickObject(arr, esize, i + 1, r, k, cmp, swap);
      }
    }
  }
  return 0;
}