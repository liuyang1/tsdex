#ifndef __FILEIN_H_
#define __FILEIN_H_

#include "ts.h"

namespace in
{
    class FileIn {
        public:
            const static int MaxLen = TS_PACKET_SIZE;
        private:
            FILE *fp;
            unsigned char tb[MaxLen];
            size_t tblen;
            bool filter();
            size_t read();
        public:
            FileIn(const char *name);
            ~FileIn();
            int loop();
    };
}
#endif
