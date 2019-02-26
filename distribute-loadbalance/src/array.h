/*************************************************************************
  > File Name: array.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdint.h>
#include <stdio.h>
typedef int (*arrayCompareCb)(const void *, const void *);
typedef int (*arrayEachCb)(void *, void *);

struct array {
    uint32_t nelem;  /* # element */
    void     *elem;  /* element */
    size_t   size;   /* element size */
    uint32_t nalloc; /* # allocated element */
};

#define nullArray { 0, NULL, 0, 0 }

void arrayNil(struct array *a);
void arraySet(struct array *a, void *elem, size_t size, uint32_t nalloc);
uint32_t arrayGetSize(const struct array *a);

struct array *arrayCreate(uint32_t n, size_t size);
void arrayDestroy(struct array *a);
int arrayInit(struct array *a, uint32_t n, size_t size);
void arrayDeinit(struct array *a);

uint32_t arrayIndex(struct array *a, void *elem);
void *arrayPush(struct array *a);
void *arrayPop(struct array *a);
void *arrayGet(struct array *a, uint32_t idx);
void *arrayTop(struct array *a);
void arraySwap(struct array *a, struct array *b);
void arraySort(struct array *a, arrayCompareCb compare);
int arrayEach(struct array *a, arrayEachCb func, void *data);

#endif
