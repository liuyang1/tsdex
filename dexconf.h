/*
 * ========================================================
 *
 *       Filename:  dexconf.h
 *
 *    Description:  对于本工程的配置分析文件
 *
 *        Version:  1.0
 *        Created:  03/13/2013 10:15:08 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#ifndef _DEXCONF_H_
#define _DEXCONF_H_
typedef struct _dex_conf_t {
    const char *LogLevel;
    const char *LogFile;
    const char *FileName;
} dex_conf_t, *pdex_conf_t;

int dex_load_conf(const char *fn);
int dex_check_conf();
int dex_print_conf();

extern dex_conf_t g_DexConf;

#endif // end of _DEXCONF_H_
