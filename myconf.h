#ifndef __MY_CONF_H__
#define __MY_CONF_H__

#include <map>
#include <string>

class CMyConf
{
    public:
        CMyConf();
        ~CMyConf();
        int load_conf(const char *fn);
        const char *get_item(const char *pkey);

    private:
        std::map < std::string, std::string > pairs;
};

#endif
