#ifndef HOME_STORE_LEVELDBIMPL_H_
#define HOME_STORE_LEVELDBIMPL_H_

#include <string>

#include <leveldb/db.h>

#include "Db.h"
#include "proto/Status.h"

namespace homestore
{

class LevelDB : public DB
{
    public:
        LevelDB(const std::string &name);
        virtual ~LevelDB();

    public:
        virtual Code Get(const std::string &key, std::string &val);
        virtual Code Put(const std::string &key, const std::string &val);
        virtual Code Del(const std::string &key);
        
    private:
        Code Init();

    private:
        std::string dbname_;
        leveldb::DB *db_;
};

}

#endif
