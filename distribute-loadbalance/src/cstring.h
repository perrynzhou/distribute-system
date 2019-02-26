/*************************************************************************
  > File Name: cstring.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/
#ifndef _CSTRING_H_
#define _CSTRING_H_

#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
struct string
{
    uint32_t len;  /* string length */
    uint8_t *data; /* string data */
};

#define string(_str)                        \
    {                                       \
        sizeof(_str) - 1, (uint8_t *)(_str) \
    }
#define nilString \
    {             \
        0, NULL   \
    }

#define stringSetText(_str, _text)                   \
    do                                               \
    {                                                \
        (_str)->len = (uint32_t)(sizeof(_text) - 1); \
        (_str)->data = (uint8_t *)(_text);           \
    } while (0);

#define stringSetRaw(_str, _raw)                \
    do                                          \
    {                                           \
        (_str)->len = (uint32_t)(strlen(_raw)); \
        (_str)->data = (uint8_t *)(_raw);       \
    } while (0);

void stringInitWithData(struct string *str,const char *s);
void stringInit(struct string *str);
void stringDeinit(struct string *str);
bool stringEmpty(const struct string *str);
int stringDuplicate(struct string *dst, const struct string *src);
int stringCopy(struct string *dst, const uint8_t *src, uint32_t srclen);
int stringCompare(const struct string *s1, const struct string *s2);
#endif
