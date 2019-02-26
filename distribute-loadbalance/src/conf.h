/*************************************************************************
  > File Name: conf.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/
#ifndef _CONF_H_
#define _CONF_H_

#include "array.h"
#include "cstring.h"
#include "util.h"
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <yaml.h>

#define CONF_OK (void *)NULL
#define CONF_ERROR (void *)"has an invalid value"

#define CONF_ROOT_DEPTH 1
#define CONF_MAX_DEPTH (CONF_ROOT_DEPTH+1)

#define CONF_DEFAULT_ARGS 3
#define CONF_DEFAULT_POOL 8

#define CONF_UNSET_NUM -1
#define CONF_UNSET_PTR NULL

#define CONF_DEFAULT_TIMEOUT -1
#define CONF_DEFAULT_LISTEN_BACKLOG 512
#define CONF_DEFAULT_PORT 5432
#define CONF_DEFAULT_HTTP_VERSION 1
#define CONF_MAX_HTTP_VERSION 2
struct confPool
{
    struct string name; /* pool name (root node) */
    int port;
    struct string addr; /* listen: */
    int threads;        /*  threads worker for accept  client connection*/
    int timeout;        /* timeout: */
    int backlog;        /* backlog: */
    struct string tags;        /*http version */
    int liveport;  /* detect server is alive port */

};

struct conf
{
    char *fname;               /* file name (ref in argv[]) */
    FILE *fh;                  /* file handle */
    struct array arg;          /* string[] (parsed {key, value} pairs) */
    struct array pool;         /* conf_pool[] (parsed pools) */
    uint32_t depth;            /* parsed tree depth */
    yaml_parser_t parser;      /* yaml parser */
    yaml_event_t event;        /* yaml event */
    yaml_token_t token;        /* yaml token */
    unsigned seq : 1;          /* sequence? */
    unsigned valid_parser : 1; /* valid parser? */
    unsigned valid_event : 1;  /* valid event? */
    unsigned valid_token : 1;  /* valid token? */
    unsigned sound : 1;        /* sound? */
    unsigned parsed : 1;       /* parsed? */
    unsigned valid : 1;        /* valid? */
};

struct command
{
    struct string name;
    char *(*set)(struct conf *cf, struct command *cmd, void *data);
    int offset;
};

#define nilCommand         \
    {                        \
        nilString, NULL, 0 \
    }

char *confSetString(struct conf *cf, struct command *cmd, void *conf);
char *confSetNum(struct conf *cf, struct command *cmd, void *conf);
char *confSetBool(struct conf *cf, struct command *cmd, void *conf);

struct conf *confCreate(const char *filename,bool is_multilevel);
void confDump(struct conf *cf);
void confDestroy(struct conf *cf);

#endif
