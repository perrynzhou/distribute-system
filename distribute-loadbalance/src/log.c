/*************************************************************************
  > File Name: log.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 五  2/22 14:25:44 2019
 ************************************************************************/

#include "log.h"
#include "util.h"
#include "cstring.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
static struct logger logger;

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    int n;

    n = vsnprintf(buf, size, fmt, args);
    if (n <= 0)
    {
        return 0;
    }

    if (n < (int)size)
    {
        return n;
    }

    return (int)(size - 1);
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = vscnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}
int logInit(int level, char *name)
{
    struct logger *l = &logger;

    l->level = MAX(LOG_EMERG_LEVEL, MIN(level, LOG_PVERB_LEVEL));
    l->name = name;
    if (name == NULL || !strlen(name))
    {
        l->fd = STDERR_FILENO;
    }
    else
    {
        l->fd = open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0)
        {
            logStderr("opening log file '%s' failed: %s", name,
                       strerror(errno));
            return -1;
        }
    }

    return 0;
}

void logDeinit(void)
{
    struct logger *l = &logger;

    if (l->fd < 0 || l->fd == STDERR_FILENO)
    {
        return;
    }

    close(l->fd);
}

void logReopen(void)
{
    struct logger *l = &logger;

    if (l->fd != STDERR_FILENO)
    {
        close(l->fd);
        l->fd = open(l->name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (l->fd < 0)
        {
            logStderrSafe("reopening log file '%s' failed, ignored: %s", l->name,
                            strerror(errno));
        }
    }
}

void logLevelUp(void)
{
    struct logger *l = &logger;

    if (l->level < LOG_PVERB_LEVEL)
    {
        l->level++;
        logSafe("up log level to %d", l->level);
    }
}

void logLevelDown(void)
{
    struct logger *l = &logger;

    if (l->level > LOG_EMERG_LEVEL)
    {
        l->level--;
        logSafe("down log level to %d", l->level);
    }
}

void logLevelSet(int level)
{
    struct logger *l = &logger;

    l->level = MAX(LOG_EMERG_LEVEL, MIN(level, LOG_PVERB_LEVEL));
    loga("set log level to %d", l->level);
}

void logStackTrace(void)
{
    struct logger *l = &logger;

    if (l->fd < 0)
    {
        return;
    }
    stacktrace_fd(l->fd);
}

int logLoggable(int level)
{
    struct logger *l = &logger;

    if (level > l->level)
    {
        return 0;
    }

    return 1;
}

void _log(const char *file, int line, int panic, const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    char buf[LOG_MAX_LEN];
    va_list args;
    ssize_t n;
    struct timeval tv;

    if (l->fd < 0)
    {
        return;
    }

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    gettimeofday(&tv, NULL);
    buf[len++] = '[';
    len += strftime(buf + len, size - len, "%Y-%m-%d %H:%M:%S.", localtime(&tv.tv_sec));
    len += scnprintf(buf + len, size - len, "%03ld", tv.tv_usec / 1000);
    len += scnprintf(buf + len, size - len, "] %s:%d ", file, line);

    va_start(args, fmt);
    len += vscnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(l->fd, buf, len);
    if (n < 0)
    {
        l->nerror++;
    }

    errno = errno_save;

    if (panic)
    {
        abort();
    }
}

void _logStderr(const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    char buf[4 * LOG_MAX_LEN];
    va_list args;
    ssize_t n;

    errno_save = errno;
    len = 0;                /* length of output buffer */
    size = 4 * LOG_MAX_LEN; /* size of output buffer */

    va_start(args, fmt);
    len += vscnprintf(buf, size, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(STDERR_FILENO, buf, len);
    if (n < 0)
    {
        l->nerror++;
    }

    errno = errno_save;
}

/*
 * Hexadecimal dump in the canonical hex + ascii display
 * See -C option in man hexdump
 */
void _logHexdump(const char *file, int line, char *data, int datalen,
                  const char *fmt, ...)
{
    struct logger *l = &logger;
    char buf[8 * LOG_MAX_LEN];
    int i, off, len, size, errno_save;
    ssize_t n;

    if (l->fd < 0)
    {
        return;
    }

    /* log hexdump */
    errno_save = errno;
    off = 0;                /* data offset */
    len = 0;                /* length of output buffer */
    size = 8 * LOG_MAX_LEN; /* size of output buffer */

    while (datalen != 0 && (len < size - 1))
    {
        char *save, *str;
        unsigned char c;
        int savelen;

        len += scnprintf(buf + len, size - len, "%08x  ", off);

        save = data;
        savelen = datalen;

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++)
        {
            c = (unsigned char)(*data);
            str = (i == 7) ? "  " : " ";
            len += scnprintf(buf + len, size - len, "%02x%s", c, str);
        }
        for (; i < 16; i++)
        {
            str = (i == 7) ? "  " : " ";
            len += scnprintf(buf + len, size - len, "  %s", str);
        }

        data = save;
        datalen = savelen;

        len += scnprintf(buf + len, size - len, "  |");

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++)
        {
            c = (unsigned char)(isprint(*data) ? *data : '.');
            len += scnprintf(buf + len, size - len, "%c", c);
        }
        len += scnprintf(buf + len, size - len, "|\n");

        off += 16;
    }

    n = write(l->fd, buf, len);
    if (n < 0)
    {
        l->nerror++;
    }

    if (len >= size - 1)
    {
        n = write(l->fd, "\n", 1);
        if (n < 0)
        {
            l->nerror++;
        }
    }

    errno = errno_save;
}

void _logSafe(const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    char buf[LOG_MAX_LEN];
    va_list args;
    ssize_t n;

    if (l->fd < 0)
    {
        return;
    }

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    len += snprintf(buf + len, size - len, "[.......................] ");

    va_start(args, fmt);
    len += vsnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(l->fd, buf, len);
    if (n < 0)
    {
        l->nerror++;
    }

    errno = errno_save;
}

void _logStderrSafe(const char *fmt, ...)
{
    struct logger *l = &logger;
    int len, size, errno_save;
    char buf[LOG_MAX_LEN];
    va_list args;
    ssize_t n;

    errno_save = errno;
    len = 0;            /* length of output buffer */
    size = LOG_MAX_LEN; /* size of output buffer */

    len += snprintf(buf + len, size - len, "[.......................] ");

    va_start(args, fmt);
    len += vsnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(STDERR_FILENO, buf, len);
    if (n < 0)
    {
        l->nerror++;
    }

    errno = errno_save;
}
