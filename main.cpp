//    Copyright [2013] <Copyright liuyang1>
/*
 * ========================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  11/21/2012 11:59:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include <stdio.h>
#include <signal.h>
#include "mylog.h"
#include "ts.h"
#include "dexconf.h"
#include "filein.h"

void *readThread(void) {
    in::FileIn input(g_DexConf.FileName);
    input.loop();
    return NULL;
}

int main() {
    dex_load_conf("dex.conf");
    dex_check_conf();
    int ret = INITLOG(g_DexConf.LogFile, logger::str2level(g_DexConf.LogLevel));
    if (ret != 0) {
        printf("init log error\n");
    }
    dex_print_conf();

    readThread();
    return 0;
}
