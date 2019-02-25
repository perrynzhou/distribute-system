/*************************************************************************
  > File Name: hashkit.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 二 11/20 12:48:06 2018
 ************************************************************************/

#ifndef _HASAHKIT_H
#define _HASAHKIT_H
#include <stdint.h>
#include <stdio.h>
uint32_t hash_crc16(const char* key, size_t key_length);
uint32_t hash_crc32(const char *key, size_t key_length);
uint32_t hash_crc32a(const char *key, size_t key_length);
uint32_t hash_fnv1_64(const char *key, size_t key_length);
uint32_t hash_hsieh(const char *key, size_t key_length);
uint32_t hash_jenkins(const char *key, size_t length);
uint32_t hash_fnv1a_64(const char *key, size_t key_length);
uint32_t hash_murmur(const char *key, size_t length);
uint32_t hash_fnv1_32(const char *key, size_t key_length);
uint32_t hash_fnv1a_32(const char *key, size_t key_length);
uint32_t hash_one_at_a_time(const char *key, size_t key_length);
uint32_t hash_jump_consistent(uint64_t key, int32_t num_buckets);
#endif
