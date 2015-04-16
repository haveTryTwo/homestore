#ifndef HOME_STORE_DB_H_
#define HOME_STORE_DB_H_

#include <string>

#include "proto/Status.h"

namespace homestore
{

class DB
{
    public:
        DB() {}
        virtual ~DB() {}

    public:
        virtual Code Get(const std::string &key, std::string &val) = 0;
        virtual Code Put(const std::string &key, const std::string &val) = 0;
        virtual Code Del(const std::string &key) = 0;
};

}

#endif
