/*************************************************************************
        > File Name: vector.h
        > Author: perrynzhou
        > Mail: perrynzhou@gmail.com
        > Created Time: Fri 02 Jun 2017 08:57:12 PM HKT
 ************************************************************************/

#ifndef _VECTOR_H
#define _VECTOR_H
#include <stdbool.h>
#include <stdint.h>
typedef struct vector_s vector;
typedef void (*free_callback) (void *data);
typedef bool (*cmp_callback) (void *data1, void *data2);	//true data1 <=data2;false data1 >data2
vector *vectorCreate (int64_t maxsize, free_callback fck, cmp_callback cck);
bool vectorAdd (vector * v, void *data);
bool vectorDel (vector * v, int64_t index);
void *vectorGet (vector * v, int64_t index);
void vectorSort (vector * v);
void vectorDestroy (vector * v);
int64_t vectorSize (vector * v);
#endif
