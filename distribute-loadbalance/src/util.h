/*************************************************************************
  > File Name: util.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:24:18 2019
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H
const char *fill = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int initSocket(int port, int backlog);
void randomString(int size, char *str);
#endif
