/*
 * ========================================================
 *
 *       Filename:  base.cpp
 *
 *    Description:  基础文件函数
 *
 *        Version:  1.0
 *        Created:  03/08/2013 06:36:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include "base.h"
#include "mylog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

bool chkDir(const char *dir)
{
    DIR *pdir = opendir(dir);
    if (NULL == pdir) {
        return false;
    } else {
        closedir(pdir);
        return true;
    }
}

bool createDir(const char *dir)
{
    if (access(dir, 0) == 0) {// exist same name file
        DIR *pdir = NULL;
        if ((pdir = opendir(dir))) {   // 文件夹已经存在
            INFO("exist dir [%s]", dir);
            return true;
        }
        ERROR("create dir [%s] failed: %s", dir, strerror(errno));
        return false;
    }
    if (mkdir(dir, 0777)) {
        ERROR("create dir [%s] failed: %s", dir, strerror(errno));
        return false;
    }
    INFO("create dir [%s]", dir);
    return true;
}
