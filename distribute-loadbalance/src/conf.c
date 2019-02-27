/*************************************************************************
  > File Name: conf.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/
#include "conf.h"
#include "log.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
static struct command conf_commands[] = {
    {string("node_addr"), confSetString, offsetof(struct confPool, nodeAddr)},
    {string("node_port"), confSetNum, offsetof(struct confPool, nodePort)},
    {string("node_tcp_timeout"), confSetNum, offsetof(struct confPool, nodeTcpTimeout)},
    {string("node_worker_threads"), confSetNum, offsetof(struct confPool, nodeWorkerThreads)},
    {string("node_tcp_backlog"), confSetNum, offsetof(struct confPool, nodeTcpBacklog)},
    {string("node_tags"), confSetString, offsetof(struct confPool, nodeTags)},
    {string("node_weight"), confSetNum, offsetof(struct confPool, nodeWeight)},
    {string("node_report_status_timeout"), confSetNum, offsetof(struct confPool, nodeReportStatusTimeout)},
    {string("cluster_addr"), confSetString, offsetof(struct confPool, clusterAddr)},
    {string("cluster_heartbeat_port"), confSetNum, offsetof(struct confPool, clusterHeartbeatPort)},
    nilCommand,
};
static int confPoolInit(struct confPool *cp, struct string *name)
{
  int status;

  stringInit(&cp->nodeName);
  cp->nodeTcpTimeout = CONF_UNSET_NUM;
  cp->nodeTcpBacklog = CONF_UNSET_NUM;
  cp->nodeWorkerThreads = CONF_UNSET_NUM;
  cp->nodePort = CONF_UNSET_NUM;
  cp->nodeReportStatusTimeout=CONF_UNSET_NUM;
  cp->nodeWeight=CONF_UNSET_NUM;
  cp->clusterHeartbeatPort=CONF_UNSET_NUM;
  stringInit(&cp->nodeTags);
  stringInit(&cp->nodeAddr);
  stringInit(&cp->clusterAddr);
  status = stringDuplicate(&cp->nodeName, name);
  if (status != 0)
  {
    return status;
  }
  stringInit(&cp->nodeAddr);
  logDebug(LOG_VVERB, "init conf pool %p, '%.*s'", cp, name->len, name->data);
  return 0;
}

static void confPoolDeinit(struct confPool *cp)
{
  stringDeinit(&cp->nodeName);
  stringDeinit(&cp->nodeAddr);
  stringDeinit(&cp->nodeTags);
  logDebug(LOG_VVERB, "deinit conf pool %p", cp);
}

void confDump(struct conf *cf)
{
  uint32_t i, j, npool, nserver;
  struct confPool *cp;
  struct string *s;

  npool = arrayGetSize(&cf->pool);
  if (npool == 0)
  {
    return;
  }

  logInfo(LOG_INFO_LEVEL, "%d  pools in configuration file '%s'", npool, cf->fname);

  for (i = 0; i < npool; i++)
  {
    cp = arrayGet(&cf->pool, i);

    logInfo(LOG_INFO_LEVEL, "%.*s", cp->nodeName.len, cp->nodeName.data);
    logInfo(LOG_INFO_LEVEL, "%.*s", cp->nodeAddr.len, cp->nodeAddr.data);
    logInfo(LOG_INFO_LEVEL, "%.*s", cp->nodeTags, cp->nodeTags.data);
    logInfo(LOG_INFO_LEVEL, "  port: %d", cp->nodePort);
    logInfo(LOG_INFO_LEVEL, "  timeout: %d", cp->nodeTcpTimeout);
    logInfo(LOG_INFO_LEVEL, "  backlog: %d", cp->nodeTcpBacklog);
    logInfo(LOG_INFO_LEVEL, "  threads: %d", cp->nodeWorkerThreads);
  }
}

static int confYamlInit(struct conf *cf)
{
  int rv;

  ASSERT(!cf->valid_parser);

  rv = fseek(cf->fh, 0L, SEEK_SET);
  if (rv < 0)
  {
    logError("conf: failed to seek to the beginning of file '%s': %s",
             cf->fname, strerror(errno));
    return -1;
  }

  rv = yaml_parser_initialize(&cf->parser);
  if (!rv)
  {
    logError("conf: failed (err %d) to initialize yaml parser",
             cf->parser.error);
    return -1;
  }

  yaml_parser_set_input_file(&cf->parser, cf->fh);
  cf->valid_parser = 1;

  return 0;
}

static void confYamlDeinit(struct conf *cf)
{
  if (cf->valid_parser)
  {
    yaml_parser_delete(&cf->parser);
    cf->valid_parser = 0;
  }
}

static int confTokenNext(struct conf *cf)
{
  int rv;

  ASSERT(cf->valid_parser && !cf->valid_token);

  rv = yaml_parser_scan(&cf->parser, &cf->token);
  if (!rv)
  {
    logError("conf: failed (err %d) to scan next token", cf->parser.error);
    return -1;
  }
  cf->valid_token = 1;

  return 0;
}

static void confTokenDone(struct conf *cf)
{
  ASSERT(cf->valid_parser);

  if (cf->valid_token)
  {
    yaml_token_delete(&cf->token);
    cf->valid_token = 0;
  }
}

static int confEventNext(struct conf *cf)
{
  int rv;

  ASSERT(cf->valid_parser && !cf->valid_event);

  rv = yaml_parser_parse(&cf->parser, &cf->event);
  if (!rv)
  {
    logError("conf: failed (err %d) to get next event", cf->parser.error);
    return -1;
  }
  cf->valid_event = 1;

  return 0;
}

static void confEventDone(struct conf *cf)
{
  if (cf->valid_event)
  {
    yaml_event_delete(&cf->event);
    cf->valid_event = 0;
  }
}

static int confPushScalar(struct conf *cf)
{
  int status;
  struct string *value;
  uint8_t *scalar;
  uint32_t scalar_len;

  scalar = cf->event.data.scalar.value;
  scalar_len = (uint32_t)cf->event.data.scalar.length;
  if (scalar_len == 0)
  {
    return -1;
  }

  logDebug(LOG_VVERB, "push '%.*s'", scalar_len, scalar);

  value = arrayPush(&cf->arg);
  if (value == NULL)
  {
    return -3;
  }
  stringInit(value);

  status = stringCopy(value, scalar, scalar_len);
  if (status != 0)
  {
    arrayPop(&cf->arg);
    return status;
  }

  return 0;
}

static void confPopScalar(struct conf *cf)
{
  struct string *value;

  value = arrayPop(&cf->arg);
  logDebug(LOG_VVERB, "pop '%.*s'", value->len, value->data);
  stringDeinit(value);
}

static int confHandler(struct conf *cf, void *data)
{
  struct command *cmd;
  struct string *key, *value;
  uint32_t narg;

  if (arrayGetSize(&cf->arg) == 1)
  {
    value = arrayTop(&cf->arg);
    logDebug(LOG_VVERB, "conf handler on '%.*s'", value->len, value->data);
    return confPoolInit(data, value);
  }

  narg = arrayGetSize(&cf->arg);
  value = arrayGet(&cf->arg, narg - 1);
  key = arrayGet(&cf->arg, narg - 2);

  logDebug(LOG_VVERB, "conf handler on %.*s: %.*s", key->len, key->data,
           value->len, value->data);

  for (cmd = conf_commands; cmd->name.len != 0; cmd++)
  {
    char *rv;

    if (stringCompare(key, &cmd->name) != 0)
    {
      continue;
    }
    rv = cmd->set(cf, cmd, data);
    if (rv != CONF_OK)
    {
      logError("conf: directive \"%.*s\" %s", key->len, key->data, rv);
      return -1;
    }

    return 0;
  }
  return 0;
}

static int confBeginParse(struct conf *cf)
{
  int status;
  bool done;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(cf->depth == 0);

  status = confYamlInit(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  do
  {
    status = confEventNext(cf);
    if (status != 0)
    {
      return status;
    }

    logDebug(LOG_VVERB, "next begin event %d", cf->event.type);

    switch (cf->event.type)
    {
    case YAML_STREAM_START_EVENT:
    case YAML_DOCUMENT_START_EVENT:
      break;

    case YAML_MAPPING_START_EVENT:
      ASSERT(cf->depth < CONF_MAX_DEPTH);
      cf->depth++;
      done = true;
      break;

    default:
      NOT_REACHED();
    }

    confEventDone(cf);

  } while (!done);

  return 0;
}

static int confEndParse(struct conf *cf)
{
  int status;
  bool done;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(cf->depth == 0);

  done = false;
  do
  {
    status = confEventNext(cf);
    if (status != 0)
    {
      return status;
    }

    logDebug(LOG_VVERB, "next end event %d", cf->event.type);

    switch (cf->event.type)
    {
    case YAML_STREAM_END_EVENT:
      done = true;
      break;

    case YAML_DOCUMENT_END_EVENT:
      break;

    default:
      NOT_REACHED();
    }

    confEventDone(cf);
  } while (!done);

  confYamlDeinit(cf);

  return 0;
}

static int confParseCore(struct conf *cf, void *data)
{
  int status;
  bool done, leaf, new_pool;

  ASSERT(cf->sound);

  status = confEventNext(cf);
  if (status != 0)
  {
    return status;
  }

  logDebug(logInfo, "next event %d depth %" PRIu32 " seq %d", cf->event.type,
           cf->depth, cf->seq);

  done = false;
  leaf = false;
  new_pool = false;

  switch (cf->event.type)
  {
  case YAML_MAPPING_END_EVENT:
    cf->depth--;
    if (cf->depth == 1)
    {
      confPopScalar(cf);
    }
    else if (cf->depth == 0)
    {
      done = true;
    }
    break;

  case YAML_MAPPING_START_EVENT:
    cf->depth++;
    break;

  case YAML_SEQUENCE_START_EVENT:
    cf->seq = 1;
    break;

  case YAML_SEQUENCE_END_EVENT:
    confPopScalar(cf);
    cf->seq = 0;
    break;

  case YAML_SCALAR_EVENT:
    status = confPushScalar(cf);
    if (status != 0)
    {
      break;
    }

    /* take appropriate action */
    if (cf->seq)
    {
      /* for a sequence, leaf is at CONF_MAX_DEPTH */
      ASSERT(cf->depth == CONF_MAX_DEPTH);
      leaf = true;
    }
    else if (cf->depth == CONF_ROOT_DEPTH)
    {
      /* create new conf_pool */
      data = arrayPush(&cf->pool);
      if (data == NULL)
      {
        status = -3;
        break;
      }
      new_pool = true;
    }
    else if (arrayGetSize(&cf->arg) == cf->depth + 1)
    {
      /* for {key: value}, leaf is at CONF_MAX_DEPTH */
      ASSERT(cf->depth == CONF_MAX_DEPTH);
      leaf = true;
    }
    break;

  default:
    NOT_REACHED();
    break;
  }

  confEventDone(cf);

  if (status != 0)
  {
    return status;
  }

  if (done)
  {
    /* terminating condition */
    return 0;
  }

  if (leaf || new_pool)
  {
    status = confHandler(cf, data);

    if (leaf)
    {
      confPopScalar(cf);
      if (!cf->seq)
      {
        confPopScalar(cf);
      }
    }

    if (status != 0)
    {
      return status;
    }
  }
  return confParseCore(cf, data);
}

static int confParse(struct conf *cf)
{
  int status;

  ASSERT(cf->sound && !cf->parsed);
  ASSERT(arrayGetSize(&cf->arg) == 0);

  status = confBeginParse(cf);
  if (status != 0)
  {
    return status;
  }

  status = confParseCore(cf, NULL);
  if (status != 0)
  {
    return status;
  }

  status = confEndParse(cf);
  if (status != 0)
  {
    return status;
  }

  cf->parsed = 1;

  return 0;
}

static struct conf *confOpen(const char *filename)
{
  int status;
  struct conf *cf;
  FILE *fh;

  fh = fopen(filename, "r");
  if (fh == NULL)
  {
    logError("conf: failed to open configuration '%s': %s", filename,
             strerror(errno));
    return NULL;
  }

  cf = calloc(1, sizeof(*cf));
  if (cf == NULL)
  {
    fclose(fh);
    return NULL;
  }

  status = arrayInit(&cf->arg, CONF_DEFAULT_ARGS, sizeof(struct string));
  if (status != 0)
  {
    free(cf);
    fclose(fh);
    return NULL;
  }

  status = arrayInit(&cf->pool, CONF_DEFAULT_POOL, sizeof(struct confPool));
  if (status != 0)
  {
    arrayDeinit(&cf->arg);
    free(cf);
    fclose(fh);
    return NULL;
  }

  cf->fname = strdup(filename);
  cf->fh = fh;
  cf->depth = 0;
  /* parser, event, and token are initialized later */
  cf->seq = 0;
  cf->valid_parser = 0;
  cf->valid_event = 0;
  cf->valid_token = 0;
  cf->sound = 0;
  cf->parsed = 0;
  cf->valid = 0;

  logDebug(LOG_VVERB, "opened conf '%s'", filename);

  return cf;
}

static int confValidateDocument(struct conf *cf)
{
  int status;
  uint32_t count;
  bool done;

  status = confYamlInit(cf);
  if (status != 0)
  {
    return status;
  }

  count = 0;
  done = false;
  do
  {
    yaml_document_t document;
    yaml_node_t *node;
    int rv;

    rv = yaml_parser_load(&cf->parser, &document);
    if (!rv)
    {
      logError("conf: failed (err %d) to get the next yaml document",
               cf->parser.error);
      confYamlDeinit(cf);
      return -1;
    }

    node = yaml_document_get_root_node(&document);
    if (node == NULL)
    {
      done = true;
    }
    else
    {
      count++;
    }

    yaml_document_delete(&document);
  } while (!done);

  confYamlDeinit(cf);

  if (count != 1)
  {
    logError("conf: '%s' must contain only 1 document; found %d documents",
             cf->fname, count);
    return -1;
  }

  return 0;
}

static int confValidateTokens(struct conf *cf)
{
  int status;
  bool done, error;
  int type;

  status = confYamlInit(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  error = false;
  do
  {
    status = confTokenNext(cf);
    if (status != 0)
    {
      return status;
    }
    type = cf->token.type;

    switch (type)
    {
    case YAML_NO_TOKEN:
      error = true;
      logError("conf: no token (%d) is disallowed", type);
      break;

    case YAML_VERSION_DIRECTIVE_TOKEN:
      error = true;
      logError("conf: version directive token (%d) is disallowed", type);
      break;

    case YAML_TAG_DIRECTIVE_TOKEN:
      error = true;
      logError("conf: tag directive token (%d) is disallowed", type);
      break;

    case YAML_DOCUMENT_START_TOKEN:
      error = true;
      logError("conf: document start token (%d) is disallowed", type);
      break;

    case YAML_DOCUMENT_END_TOKEN:
      error = true;
      logError("conf: document end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_SEQUENCE_START_TOKEN:
      error = true;
      logError("conf: flow sequence start token (%d) is disallowed", type);
      break;

    case YAML_FLOW_SEQUENCE_END_TOKEN:
      error = true;
      logError("conf: flow sequence end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_MAPPING_START_TOKEN:
      error = true;
      logError("conf: flow mapping start token (%d) is disallowed", type);
      break;

    case YAML_FLOW_MAPPING_END_TOKEN:
      error = true;
      logError("conf: flow mapping end token (%d) is disallowed", type);
      break;

    case YAML_FLOW_ENTRY_TOKEN:
      error = true;
      logError("conf: flow entry token (%d) is disallowed", type);
      break;

    case YAML_ALIAS_TOKEN:
      error = true;
      logError("conf: alias token (%d) is disallowed", type);
      break;

    case YAML_ANCHOR_TOKEN:
      error = true;
      logError("conf: anchor token (%d) is disallowed", type);
      break;

    case YAML_TAG_TOKEN:
      error = true;
      logError("conf: tag token (%d) is disallowed", type);
      break;

    case YAML_BLOCK_SEQUENCE_START_TOKEN:
    case YAML_BLOCK_MAPPING_START_TOKEN:
    case YAML_BLOCK_END_TOKEN:
    case YAML_BLOCK_ENTRY_TOKEN:
      break;

    case YAML_KEY_TOKEN:
    case YAML_VALUE_TOKEN:
    case YAML_SCALAR_TOKEN:
      break;

    case YAML_STREAM_START_TOKEN:
      break;

    case YAML_STREAM_END_TOKEN:
      done = true;
      logDebug(LOG_VVERB, "conf '%s' has valid tokens", cf->fname);
      break;

    default:
      error = true;
      logError("conf: unknown token (%d) is disallowed", type);
      break;
    }

    confTokenDone(cf);
  } while (!done && !error);

  confYamlDeinit(cf);

  return !error ? 0 : -1;
}

static int confValidateStructure(struct conf *cf)
{
  int status;
  int type, depth;
  uint32_t i, count[CONF_MAX_DEPTH + 1];
  bool done, error, seq;

  status = confYamlInit(cf);
  if (status != 0)
  {
    return status;
  }

  done = false;
  error = false;
  seq = false;
  depth = 0;
  for (i = 0; i < CONF_MAX_DEPTH + 1; i++)
  {
    count[i] = 0;
  }

  /*
   * Validate that the configuration conforms roughly to the following
   * yaml tree structure:
   *
   * keyx:
   *   key1: value1
   *   key2: value2
   *   seq:
   *     - elem1
   *     - elem2
   *     - elem3
   *   key3: value3
   *
   * keyy:
   *   key1: value1
   *   key2: value2
   *   seq:
   *     - elem1
   *     - elem2
   *     - elem3
   *   key3: value3
   */
  do
  {
    status = confEventNext(cf);
    if (status != 0)
    {
      return status;
    }

    type = cf->event.type;

    logDebug(LOG_VVERB, "next event %d depth %d seq %d", type, depth, seq);

    switch (type)
    {
    case YAML_STREAM_START_EVENT:
    case YAML_DOCUMENT_START_EVENT:
      break;

    case YAML_DOCUMENT_END_EVENT:
      break;

    case YAML_STREAM_END_EVENT:
      done = true;
      break;

    case YAML_MAPPING_START_EVENT:
      if (depth == CONF_ROOT_DEPTH && count[depth] != 1)
      {
        error = true;
        logError("conf: '%s' has more than one \"key:value\" at depth"
                 " %d",
                 cf->fname, depth);
      }
      else if (depth >= CONF_MAX_DEPTH)
      {
        error = true;
        logError("conf: '%s' has a depth greater than %d", cf->fname,
                 CONF_MAX_DEPTH);
      }
      depth++;
      break;

    case YAML_MAPPING_END_EVENT:
      if (depth == CONF_MAX_DEPTH)
      {
        if (seq)
        {
          seq = false;
        }
        else
        {
          error = true;
          logError("conf: '%s' missing sequence directive at depth "
                   "%d",
                   cf->fname, depth);
        }
      }
      depth--;
      count[depth] = 0;
      break;

    case YAML_SEQUENCE_START_EVENT:
      if (seq)
      {
        error = true;
        logError("conf: '%s' has more than one sequence directive", cf->fname);
      }
      else if (depth != CONF_MAX_DEPTH)
      {
        error = true;
        logError("conf: '%s' has sequence at depth %d instead of %d",
                 cf->fname, depth, CONF_MAX_DEPTH);
      }
      else if (count[depth] != 1)
      {
        error = true;
        logError("conf: '%s' has invalid \"key:value\" at depth %d", cf->fname,
                 depth);
      }
      seq = true;
      break;

    case YAML_SEQUENCE_END_EVENT:
      ASSERT(depth == CONF_MAX_DEPTH);
      count[depth] = 0;
      break;

    case YAML_SCALAR_EVENT:
      if (depth == 0)
      {
        error = true;
        logError("conf: '%s' has invalid empty \"key:\" at depth %d",
                 cf->fname, depth);
      }
      else if (depth == CONF_ROOT_DEPTH && count[depth] != 0)
      {
        error = true;
        logError("conf: '%s' has invalid mapping \"key:\" at depth %d",
                 cf->fname, depth);
      }
      else if (depth == CONF_MAX_DEPTH && count[depth] == 2)
      {
        /* found a "key: value", resetting! */
        count[depth] = 0;
      }
      count[depth]++;
      break;

    default:
      NOT_REACHED();
    }

    confEventDone(cf);
  } while (!done && !error);

  confYamlDeinit(cf);

  return !error ? 0 : -1;
}

static int confPreValidate(struct conf *cf, bool is_multilevel)
{
  int status;

  status = confValidateDocument(cf);
  if (status != 0)
  {
    return status;
  }

  status = confValidateTokens(cf);
  if (status != 0)
  {
    return status;
  }

  /* invalid check yaml structure */

  if (is_multilevel)
  {
    status = confValidateStructure(cf);
    if (status != 0)
    {
      return status;
    }
  }
  cf->sound = 1;

  return 0;
}

static int confPoolNameCmp(const void *t1, const void *t2)
{
  const struct confPool *p1 = t1, *p2 = t2;

  return stringCompare(&p1->nodeName, &p2->nodeName);
}

static int confPoolAddrCmp(const void *t1, const void *t2)
{
  const struct confPool *p1 = t1, *p2 = t2;

  return stringCompare(&p1->nodeAddr, &p2->nodeAddr);
}

static int confValidatePool(struct conf *cf, struct confPool *cp)
{
  int status;

  ASSERT(!stringEmpty(&cp->name));

  if (cp->nodeTcpTimeout == CONF_UNSET_NUM)
  {
    cp->nodeTcpTimeout = CONF_DEFAULT_TIMEOUT;
  }
  if (cp->nodeTcpBacklog == CONF_UNSET_NUM)
  {
    cp->nodeTcpBacklog = CONF_DEFAULT_LISTEN_BACKLOG;
  }
  if (cp->nodePort == CONF_UNSET_NUM)
  {
    cp->nodePort = CONF_DEFAULT_PORT;
  }
  return 0;
}

static int confPostValidate(struct conf *cf)
{
  int status;
  uint32_t i, npool;
  bool valid;

  ASSERT(cf->sound && cf->parsed);
  ASSERT(!cf->valid);

  npool = arrayGetSize(&cf->pool);
  if (npool == 0)
  {
    logError("conf: '%.*s' has no pools", cf->fname);
    return -1;
  }

  /* validate pool */
  for (i = 0; i < npool; i++)
  {
    struct confPool *cp = arrayGet(&cf->pool, i);

    status = confValidatePool(cf, cp);
    if (status != 0)
    {
      return status;
    }
  }

  /* disallow pools with duplicate listen: key values */
  arraySort(&cf->pool, confPoolAddrCmp);
  for (valid = true, i = 0; i < npool - 1; i++)
  {
    struct confPool *p1, *p2;

    p1 = arrayGet(&cf->pool, i);
    p2 = arrayGet(&cf->pool, i + 1);

    if (stringCompare(&p1->nodeAddr, &p2->nodeAddr) == 0 && p1->nodePort == p2->nodePort)
    {
      logError("conf: pools '%.*s' and '%.*s' have the same listen address '%s:%d'",
               p1->nodeName.len, p1->nodeName.data, p2->nodeName.len, p2->nodeName.data,
               p1->nodeAddr.data, p1->nodePort);
      valid = false;
      break;
    }
  }
  if (!valid)
  {
    return -1;
  }

  /* disallow pools with duplicate names */
  arraySort(&cf->pool, confPoolNameCmp);
  for (valid = true, i = 0; i < npool - 1; i++)
  {
    struct confPool *p1, *p2;

    p1 = arrayGet(&cf->pool, i);
    p2 = arrayGet(&cf->pool, i + 1);

    if (stringCompare(&p1->nodeName, &p2->nodeName) == 0)
    {
      logError("conf: '%s' has pools with same name %.*s'", cf->fname,
               p1->nodeName.len, p1->nodeName.data);
      valid = false;
      break;
    }
  }
  if (!valid)
  {
    return -1;
  }

  return 0;
}

struct conf *confCreate(const char *filename, bool is_multilevel)
{
  int status;
  struct conf *cf;

  cf = confOpen(filename);
  if (cf == NULL)
  {
    return NULL;
  }

  /* validate configuration file before parsing */
  status = confPreValidate(cf, is_multilevel);

  if (status != 0)
  {
    goto error;
  }

  /* parse the configuration file */
  status = confParse(cf);
  if (status != 0)
  {
    goto error;
  }

  /* validate parsed configuration */
  status = confPostValidate(cf);
  if (status != 0)
  {
    goto error;
  }

  logLevelSet(LOG_DEBUG_LEVEL);

  fclose(cf->fh);
  cf->fh = NULL;

  return cf;

error:
  logStderr("proxy: configuration file '%s' syntax is invalid", filename);
  fclose(cf->fh);
  cf->fh = NULL;
  confDestroy(cf);
  return NULL;
}

void confDestroy(struct conf *cf)
{
  while (arrayGetSize(&cf->arg) != 0)
  {
    confPopScalar(cf);
  }
  arrayDeinit(&cf->arg);

  while (arrayGetSize(&cf->pool) != 0)
  {
    confPoolDeinit(arrayPop(&cf->pool));
  }
  arrayDeinit(&cf->pool);

  free(cf);
}

char *confSetString(struct conf *cf, struct command *cmd, void *conf)
{
  int status;
  uint8_t *p;
  struct string *field, *value;

  p = conf;
  field = (struct string *)(p + cmd->offset);

  if (field->data != CONF_UNSET_PTR)
  {
    return "is a duplicate";
  }

  value = arrayTop(&cf->arg);

  status = stringDuplicate(field, value);
  if (status != 0)
  {
    return CONF_ERROR;
  }

  return CONF_OK;
}

char *confSetNum(struct conf *cf, struct command *cmd, void *conf)
{
  uint8_t *p;
  int num, *np;
  struct string *value;

  p = conf;
  np = (int *)(p + cmd->offset);

  if (*np != CONF_UNSET_NUM)
  {
    return "is a duplicate";
  }

  value = arrayTop(&cf->arg);

  num = strToi(value->data, value->len);
  if (num < 0)
  {
    return "is not a number";
  }

  *np = num;

  return CONF_OK;
}

char *confSetBool(struct conf *cf, struct command *cmd, void *conf)
{
  uint8_t *p;
  int *bp;
  struct string *value, true_str, false_str;

  p = conf;
  bp = (int *)(p + cmd->offset);

  if (*bp != CONF_UNSET_NUM)
  {
    return "is a duplicate";
  }

  value = arrayTop(&cf->arg);
  stringSetText(&true_str, "true");
  stringSetText(&false_str, "false");

  if (stringCompare(value, &true_str) == 0)
  {
    *bp = 1;
  }
  else if (stringCompare(value, &false_str) == 0)
  {
    *bp = 0;
  }
  else
  {
    return "is not \"true\" or \"false\"";
  }

  return CONF_OK;
}

char *confSetHash(struct conf *cf, struct command *cmd, void *conf)
{
  int status;
  uint8_t *p;
  struct string *field, *value;

  p = conf;
  field = (struct string *)(p + cmd->offset);

  if (field->data != CONF_UNSET_PTR)
  {
    return "is a duplicate";
  }

  value = arrayTop(&cf->arg);

  status = stringDuplicate(field, value);
  if (status != 0)
  {
    return CONF_ERROR;
  }

  return CONF_OK;
}
#ifdef CONF_TEST
int main(void)
{
  logInit(LOG_INFO_LEVEL, NULL);
  struct conf *f = confCreate("./conf.yaml", false);
  if (f != NULL)
  {
    confDump(f);
    confDestroy(f);
  }
}
#endif