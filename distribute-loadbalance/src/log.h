/*************************************************************************
  > File Name: log.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/
#ifndef _LOG_H_
#define _LOG_H_

struct logger
{
    char *name; /* log file name */
    int level;  /* log level */
    int fd;     /* log file descriptor */
    int nerror; /* # log error */
};

#define LOG_EMERG_LEVEL 0   /* system in unusable */
#define LOG_ALERT_LEVEL 1   /* action must be taken immediately */
#define LOG_CRIT_LEVEL 2    /* critical conditions */
#define LOG_ERR_LEVEL 3     /* error conditions */
#define LOG_WARNING_LEVEL 4     /* warning conditions */
#define LOG_NOTICE_LEVEL 5  /* normal but significant condition (default) */
#define LOG_INFO_LEVEL 6     /* informational */
#define LOG_DEBUG_LEVEL 7    /* debug messages */
#define LOG_VERB_LVEL 8    /* verbose messages */
#define LOG_VVERB_LEVEL 9   /* verbose messages on crack */
#define LOG_VVVERB_LEVEL 10 /* verbose messages on ganga */
#define LOG_PVERB_LEVEL 11  /* periodic verbose messages on crack */

#define LOG_MAX_LEN 256 /* max length of log message */

/*
 * logStderr   - log to stderr
 * loga         - log always
 * logaHexdump - log hexdump always
 * logError    - error log messages
 * logWarn     - warning log messages
 * logPanic    - log messages followed by a panic
 * ...
 * logDebug    - debug log messages based on a log level
 * logHexdump  - hexadump -C of a log buffer
 */
#ifdef _DEBUG_LOG_MODE

#define logDebug(_level, ...)                         \
    do                                                \
    {                                                 \
        if (logLoggable(_level) != 0)                 \
        {                                             \
            _log(__FILE__, __LINE__, 0, __VA_ARGS__); \
        }                                             \
    } while (0)

#define logHexdump(_level, _data, _datalen, ...)                              \
    do                                                                        \
    {                                                                         \
        if (logLoggable(_level) != 0)                                         \
        {                                                                     \
            _log(__FILE__, __LINE__, 0, __VA_ARGS__);                         \
            _logHexdump(__FILE__, __LINE__, (char *)(_data), (int)(_datalen), \
                        __VA_ARGS__);                                         \
        }                                                                     \
    } while (0)

#else

#define logDebug(_level, ...)
#define logHexdump(_level, _data, _datalen, ...)

#endif

#define logInfo(_level, ...)                          \
    do                                                \
    {                                                 \
        if (logLoggable(_level) != 0)                 \
        {                                             \
            _log(__FILE__, __LINE__, 0, __VA_ARGS__); \
        }                                             \
    } while (0)

#define logStderr(...)           \
    do                           \
    {                            \
        _logStderr(__VA_ARGS__); \
    } while (0)

#define logSafe(...)           \
    do                         \
    {                          \
        _logSafe(__VA_ARGS__); \
    } while (0)

#define logStderrSafe(...)           \
    do                               \
    {                                \
        _logStderrSafe(__VA_ARGS__); \
    } while (0)

#define loga(...)                                 \
    do                                            \
    {                                             \
        _log(__FILE__, __LINE__, 0, __VA_ARGS__); \
    } while (0)

#define logaHexdump(_data, _datalen, ...)                                 \
    do                                                                    \
    {                                                                     \
        _log(__FILE__, __LINE__, 0, __VA_ARGS__);                         \
        _logHexdump(__FILE__, __LINE__, (char *)(_data), (int)(_datalen), \
                    __VA_ARGS__);                                         \
    } while (0)

#define logError(...)                                 \
    do                                                \
    {                                                 \
        if (logLoggable(LOG_ALERT_LEVEL) != 0)              \
        {                                             \
            _log(__FILE__, __LINE__, 0, __VA_ARGS__); \
        }                                             \
    } while (0)

#define logWarn(...)                                  \
    do                                                \
    {                                                 \
        if (logLoggable(logWarn) != 0)                \
        {                                             \
            _log(__FILE__, __LINE__, 0, __VA_ARGS__); \
        }                                             \
    } while (0)

#define logPanic(...)                                 \
    do                                                \
    {                                                 \
        if (logLoggable(LOG_EMERG) != 0)              \
        {                                             \
            _log(__FILE__, __LINE__, 1, __VA_ARGS__); \
        }                                             \
    } while (0)

int logInit(int level, char *filename);
void logDeinit(void);
void logLevelUp(void);
void logLevelDown(void);
void logLevelSet(int level);
void logStackTrace(void);
void logReopen(void);
int logLoggable(int level);
void _log(const char *file, int line, int panic, const char *fmt, ...);
void _logStderr(const char *fmt, ...);
void _logSafe(const char *fmt, ...);
void _logStderrSafe(const char *fmt, ...);
void _logHexdump(const char *file, int line, char *data, int datalen, const char *fmt, ...);

#endif
