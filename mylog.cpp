#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include "mylog.h"

logger g_logger;

logger::logger()
{
    logfp = NULL;
    logfile = NULL;
    loglevel = LOG_LEVEL_DEFAULT;
    pthread_mutex_init(&lock, NULL);
}

logger::~logger()
{
    this->close();
    if (NULL != logfile) {
        free(logfile);
        logfile = NULL;
    }
    pthread_mutex_destroy(&lock);
}

int logger::log(my_log_level level, const char *fmt, ...)
{
    if (level < loglevel) {
        return 0;
    }
    va_list ap;
    time_t t;
    struct tm tminfo;

    t = time(NULL);
    localtime_r(&t, &tminfo);
    if (logfp == NULL) {
        logfp = stdout;
    }
#define OUTBUFLEN       2048
    static char outbuf[OUTBUFLEN];
    pthread_mutex_lock(&lock);
    {
        int n = snprintf(outbuf, OUTBUFLEN, "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d] [%s] ", 1900 +
                         tminfo.tm_year,
                         tminfo.tm_mon + 1, tminfo.tm_mday, tminfo.tm_hour, tminfo.tm_min,
                         tminfo.tm_sec, level2str(level));
        va_start(ap, fmt);
        n += vsnprintf(outbuf + n, OUTBUFLEN - n, fmt, ap);
        va_end(ap);
        fprintf(logfp, "%s\n", outbuf);
    }
    pthread_mutex_unlock(&lock);

    return 0;
}

int logger::init(const char *file, my_log_level level)
{
    if (NULL == file) {
        printf("[log] not indicate log file\n");
        return -1;
    }
    if (level <= LOG_MAX || level >= LOG_NONE) {
        printf("[log] log level wrong %d %s\n", level, level2str(level));
        return -1;
    }
    if (logfile == NULL) {
        logfile = strdup(file);
    }
    if (NULL == logfile) {
        printf("[log] copy log file name fail\n");
        return -1;
    }
    loglevel = level;
    if (file[strlen(file) - 1] == '-') {
        logfp = stdout;
    } else {
        logfp = fopen(file, "a+");
        if (NULL == logfp) {
            logfp = stdout;            // 默认输出到标准输出stdout
            return -1;
        } else {
            setvbuf(logfp, (char *)NULL, _IOLBF, 0);
        }
    }
    return 0;
}

int logger::rotate()
{
    int ret;
    char newfn[1024];

    if (logfp == stdout) {
        return 0;
    }
    ret = snprintf(newfn, sizeof newfn, "%s.%ld", logfile, time(NULL));//typedef time_t long
    if (ret >= (int)sizeof(newfn)) {
        return -1;
    }
    pthread_mutex_lock(&lock);
    fclose(logfp);
    logfp = NULL;
    rename(logfile, newfn);
    this->init(logfile, loglevel);
    pthread_mutex_unlock(&lock);

    return 0;
}

int logger::close()
{
    flush();
    if (logfp && logfp != stdout) {
        fclose(logfp);
    }
    logfp = stdout;
    return 0;
}

my_log_level logger::str2level(const char *str)
{
    if (strcmp(str, "LOG_MAX") == 0) {
        return LOG_MAX;
    } else if (strcmp(str, "LOG_DEBUG") == 0) {
        return LOG_DEBUG;
    } else if (strcmp(str, "LOG_DUMP") == 0) {
        return LOG_DUMP;
    } else if (strcmp(str, "LOG_INFO") == 0) {
        return LOG_INFO;
    } else if (strcmp(str, "LOG_NOTICE") == 0) {
        return LOG_NOTICE;
    } else if (strcmp(str, "LOG_WARN") == 0) {
        return LOG_WARN;
    } else if (strcmp(str, "LOG_ERROR") == 0) {
        return LOG_ERROR;
    } else if (strcmp(str, "LOG_FATAL") == 0) {
        return LOG_FATAL;
    } else {
        return LOG_NONE;
    }
}

void dumpBin(unsigned char *data, int len, int offset)
{
    // 为了输出不会紊乱,这里和日志输出采用相同的锁
    g_logger.locklog();
    const int linenum = 16, chunknum = 4;
    for (int i = 0; i < len; i++) {
        if (i % linenum == 0) {
            printf("%7d ", i);
        }
        printf(" %02x", *(data + offset + i));
        if (i % linenum == linenum - 1 || i == len - 1) {
            printf("\n");
        } else if ((i + 1) % chunknum == 0) {
            printf("  ");
        }
    }
    g_logger.unlocklog();
}
