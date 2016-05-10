#include "mylog.h"
#include "filein.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>

using namespace in;
extern class GlobalInfo g_Info;

FileIn::FileIn(const char *fn)
{
    fp = fopen(fn, "r");
}

FileIn::~FileIn(void)
{
    if (fp != NULL) {
        ::fclose(fp);
        fp = NULL;
    }
}

bool FileIn::filter()
{
    return 1;
}

size_t FileIn::read()
{
    ::bzero(tb, MaxLen);
    size_t ret = ::fread(tb, TS_PACKET_SIZE, 1, fp);
    return ret;
}

int FileIn::loop(void)
{
    bool ret;
    int cnt = 0;
    while (true) {
        ret = read();
        if (ret == false) {
            break;
        }
        g_tsparser.pushData(tb, TS_PACKET_SIZE);
        cnt++;
        WARN("test %d!", cnt);
        if (cnt == 3) {
            break;
        }
    }
    return 0;
}
