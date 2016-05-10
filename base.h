/*
 * ========================================================
 *
 *       Filename:  base.h
 *
 *    Description:  base 类型
 *
 *        Version:  1.0
 *        Created:  11/23/2012 11:41:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

typedef unsigned int uint;
typedef unsigned char uchar;

typedef unsigned short ushort;
typedef unsigned long long u64;
typedef signed long long s64;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohll(x)               (((long long)(ntohl((int)((x << 32) >> 32))) << 32) | (unsigned   \
                                                                                       int)ntohl( \
                                     ((int)(x >> 32))))
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(x)               (x)
#endif

#define htonll(x)               ntohll(x)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &);            \
    void operator = (const TypeName &);

bool chkDir(const char *dir);
bool createDir(const char *dir);

inline bool lock_set(int fd, int type)
{
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_type = type;
    if (fcntl(fd, F_SETLK, &lock) == 0) {
        return true;
    }
    fcntl(fd, F_GETLK, &lock);
    if (lock.l_type != F_UNLCK) {
        return false;
    }
    return false;
}

inline bool lockFile(FILE *fp) {
    int fd = fileno(fp);
    for (int i = 0; i < 3; i++) {
        if (lock_set(fd, F_WRLCK)) {
            return true;
        } else {
            usleep(200);
        }
    }
    return false;
}

inline bool unlockFile(FILE *fp) {
    int fd = fileno(fp);
    lock_set(fd, F_UNLCK);
    return true;
}

#endif
