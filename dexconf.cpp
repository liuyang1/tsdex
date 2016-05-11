/*
 * ========================================================
 *
 *       Filename:  dexconf.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/13/2013 10:18:49 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include "mylog.h"
#include "myconf.h"
#include "dexconf.h"
#include <stdlib.h>
#include <string.h>

dex_conf_t g_DexConf;

#define DEX_DEF_LOG_LEVEL       (logger::level2str(LOG_LEVEL_DEFAULT))
#define DEX_DEF_LOG_FILE        "-"

int dex_load_conf(const char *fn)
{
    CMyConf myconf;
    if (NULL == fn) {
        return -1;
    }
    g_DexConf.FileName = "";
    g_DexConf.LogLevel = DEX_DEF_LOG_LEVEL;
    g_DexConf.LogFile = DEX_DEF_LOG_FILE;

    if (0 != myconf.load_conf(fn)) {
        return -1;
    }
    const char *pval;
    pval = myconf.get_item("LogLevel");
    if (NULL != pval) {
        g_DexConf.LogLevel = strdup(pval);
    }
    pval = myconf.get_item("LogFile");
    if (NULL != pval) {
        g_DexConf.LogFile = strdup(pval);
    }
    pval = myconf.get_item("FileName");
    if (NULL != pval) {
        g_DexConf.FileName = strdup(pval);
    }
    return 0;
}

int dex_check_conf()
{
    if (g_DexConf.LogFile == NULL) {
        ERROR("[conf] bad log file NULL");
        return -1;
    }
    if (g_DexConf.LogLevel == NULL) {
        ERROR("[conf] bad log level");
        return -1;
    }
    return 0;
}

int dex_print_conf() {
    NOTICE("[conf] LogLevel  %s", g_DexConf.LogLevel);
    NOTICE("[conf] LogFile   %s", g_DexConf.LogFile);
    NOTICE("[conf] FileName  %s", g_DexConf.FileName);
    return 0;
}
