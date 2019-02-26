/*************************************************************************
  > File Name: dict.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 13:50:44 2019
 ************************************************************************/

#ifndef _DICT_H
#define _DICT_H
#include <cstring.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
typedef uint64_t (*hashCb)(void *ptr, size_t len);
typedef void (*destroyCb)(void *ptr);
enum dictErrCode
{
  success = 0,
  nilPointerErr = -1,
  keyNotExistsErr = -2,
};
typedef struct dictEntry
{
  struct string key;
  void *value;
  uint64_t keyCode;
  struct dictEntry *next;
} dictEntry;
typedef struct dict
{
  dictEntry **tables;
  int dictSlotSize;
  hashCb hashcb;
  destroyCb destroycb;
  pthread_mutex_t mutex;
} dict;

dictEntry *dictEntryCreate(const char *key, void *value, uint64_t keyCode);
void dictEntryDestroy(dictEntry *entry, destroyCb destroycb);
dict *dictCreate(int size, hashCb hashCb, destroyCb destroycb);
int dictAdd(dict *dt, void *key, void *value);
int dictDel(dict *dt, void *key);
void *dictFind(dict *dt, const char *key);
void dictDestroy(dict *dt);
#endif
