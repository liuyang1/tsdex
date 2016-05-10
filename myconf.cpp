#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "myconf.h"
#include "mylog.h"

CMyConf::CMyConf()
{
}

CMyConf::~CMyConf()
{
}

int CMyConf::load_conf(const char *fn)
{
    int ret = 0;
    char buf[1024];
    FILE *fp = NULL;
    char *p1;
    char *p2;
    char *p3;
    int len;

    if (fn == NULL) {
        return -1;
    }
    fp = fopen(fn, "r");
    if (NULL == fp) {
        ERROR("open file failed: %s", fn);
        ret = -1;
        goto _EXIT;
    }
    while (NULL != fgets(buf, sizeof(buf), fp)) {
        len = strlen(buf);
        if (len <= 0) {
            continue;
        }
        p1 = buf;
        while (*p1) {
            if (*p1 != '\r' && *p1 != '\n' && *p1 != ' ' && *p1 != '\t') {
                break;
            }
            ++p1;
        }
        p2 = buf + len - 1;
        while (p2 > p1) {
            if (*p2 == '\r' || *p2 == '\n' || *p2 == ' ' || *p2 == '\t') {
                *p2 = 0;
            } else {
                break;
            }
            --p2;
        }
        if (strlen(p1) <= 0) {
            continue;
        }
        if (p1[0] == '#') {
            continue;
        }
        p2 = strchr(p1, '=');
        if (NULL == p2) {
            WARN("bad conf item(missing =):%s", p1);
            continue;
        }
        *p2 = 0;
        p3 = p2 + 1;
        --p2;
        while (p2 > p1) {
            if (*p2 == '\r' || *p2 == '\n' || *p2 == ' ' || *p2 == '\t') {
                *p2 = 0;
            } else {
                break;
            }
            --p2;
        }
        while (*p3) {
            if (*p3 != '\r' && *p3 != '\n' && *p3 != ' ' && *p3 != '\t') {
                break;
            }
            ++p3;
        }
        if (strlen(p1) <= 0) {
            WARN("bad conf item(missing key)");
            continue;
        }
        if (strlen(p3) <= 0) {
            WARN("bad conf item(missing value)");
            continue;
        }
        pairs[p1] = p3;
        //g_logger.log(MY_LOG_DEBUG,"conf item, key:[%s],value:[%s]\n",p1,p3);
    }
_EXIT:
    if (NULL != fp) {
        fclose(fp);
    }
    return ret;
}

const char *CMyConf::get_item(const char *pkey)
{
    if (NULL == pkey) {
        return NULL;
    }
    if (pairs[pkey].length() > 0) {
        return pairs[pkey].c_str();
    }
    return NULL;
}
