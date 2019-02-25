/*************************************************************************
  > File Name: dict.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 一  2/25 13:50:49 2019
 ************************************************************************/

#include "dict.h"
#include "hashkit.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define dict_DEFAULT_SIZE (512)
dictEntry *dictEntryCreate(const char *key, void *value, uint64_t keyCode)
{
  dictEntry *entry = (dictEntry *)calloc(1, sizeof(*entry));
  assert(entry != NULL);
  entry->value = value;
  entry->key = strdup(key);
  entry->keyCode = keyCode;
  entry->next = NULL;
  return entry;
}
void dictEntryDestroy(dictEntry *entry, destroyCb destroyCb)
{
  if (entry != NULL)
  {
    free(entry->key);
    if (destroyCb != NULL)
    {
      destroyCb(entry->value);
    }
    free(entry);
    entry = NULL;
  }
}
dict *dictCreate(int size, hashCb hashcb, destroyCb destroycb)
{
  if (hashcb == NULL || destroycb == NULL)
  {
    return NULL;
  }
  dict *dt = (dict *)calloc(1, sizeof(*dt));
  assert(dt != NULL);
  dt->destroycb = destroycb;
  dt->hashcb = hashcb;
  dt->dictSlotSize = (size <= 0) ? dict_DEFAULT_SIZE : size;
  dt->tables = (dictEntry **)calloc(dt->dictSlotSize, sizeof(dictEntry *));
  return dt;
}
int dictAdd(dict *dt, void *key, void *value)
{
  if (key == NULL || value == NULL)
  {
    return nilPointerErr;
  }
  char *strKey = (char *)key;
  uint64_t keyCode = dt->hashcb(strKey, strlen(strKey));
  dictEntry *entry = dictEntryCreate(strKey, value, keyCode);
  int index = hash_jump_consistent(keyCode, dt->dictSlotSize);
  pthread_mutex_lock(&dt->mutex);
  if (dt->tables[index] == NULL)
  {
    dt->tables[index] = entry;
  }
  else
  {
    entry->next = dt->tables[index];
    dt->tables[index] = entry;
  }
  pthread_mutex_unlock(&dt->mutex);
  return success;
}

int dictDel(dict *dt, void *key)
{
  if (dt == NULL || key == NULL)
  {
    return nilPointerErr;
  }
  char *strKey = (char *)key;
  size_t strLen = strlen(strKey);
  uint64_t keyCode = dt->hashcb(strKey, strLen);
  int index = hash_jump_consistent(keyCode, dt->dictSlotSize);
  if (dt->tables[index] == NULL)
  {
    return keyNotExistsErr;
  }
  dictEntry *cur = dt->tables[index];
  dictEntry *target = NULL, *save = NULL;
  while (cur != NULL)
  {
    dictEntry *next = cur->next;
    if (strncmp(strKey, cur->key, strLen) == 0)
    {
      target = cur;
      break;
    }
    else
    {
      dictEntry *save = cur;
    }
  }
  if (target == NULL)
  {
    return keyNotExistsErr;
  }
  pthread_mutex_lock(&dt->mutex);
  save->next = target->next;
  pthread_mutex_unlock(&dt->mutex);
  return success;
}
void *dictFind(dict *dt, const char *key)
{
  if (dt == NULL || key == NULL)
  {
    return nilPointerErr;
  }
  char *strKey = (char *)key;
  size_t strLen = strlen(strKey);
  uint64_t keyCode = dt->hashcb(strKey, strLen);
  int index = hash_jump_consistent(keyCode, dt->dictSlotSize);
  if (dt->tables[index] == NULL)
  {
    return keyNotExistsErr;
  }
  dictEntry *cur = dt->tables[index];
  while (cur != NULL)
  {
    dictEntry *next = cur->next;
    if (strncmp(strKey, cur->key, strLen) == 0)
    {
      return cur->value;
    }
    cur = next;
  }
  return NULL;
}