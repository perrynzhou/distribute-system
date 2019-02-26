/*************************************************************************
  > File Name: util.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:24:18 2019
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H
#include <stdint.h>
#include <stdio.h>
const char *fill = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int strToi(uint8_t *line, size_t n);
int initSocket(int port, int backlog);
void randomString(int size, char *str);
#endif
