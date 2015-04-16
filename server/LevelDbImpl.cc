#include "LevelDbImpl.h"

namespace homestore
{

LevelDB::LevelDB(const std::string &name) : dbname_(name)
{
    Init();
}

LevelDB::~LevelDB()
{
    if (db_ != NULL) 
    {
        delete db_;
        db_ = NULL;
    }
}

Code LevelDB::Init()
{
    leveldb::Status status;
    leveldb::Options opt;
    opt.create_if_missing = true;

    status = db_->Open(opt, dbname_, &db_);
    assert(status.ok());

    return H_OK;
}

Code LevelDB::Get(const std::string &key, std::string &val)
{
    leveldb::Status status;
    leveldb::ReadOptions r_opt;
    r_opt.verify_checksums = true;

    status = db_->Get(r_opt, key, &val);

    if (status.ok()) return H_OK;
    if (status.IsNotFound()) return H_ERR_NOT_EXIST;
    if (status.IsIOError()) return H_ERR_IO;
    return H_ERR_OTHER;
}

Code LevelDB::Put(const std::string &key, const std::string &val)
{
    leveldb::Status status;
    leveldb::WriteOptions w_opt;
    w_opt.sync = true;

    status = db_->Put(w_opt, key, val);
    if (status.ok()) return H_OK;
    if (status.IsIOError()) return H_ERR_IO;
    return H_ERR_OTHER;
}

Code LevelDB::Del(const std::string &key)
{
    leveldb::Status status;
    leveldb::WriteOptions w_opt;
    w_opt.sync = true;

    status = db_->Delete(w_opt, key);
    if (status.ok()) return H_OK;
    if (status.IsIOError()) return H_ERR_IO;
    return H_ERR_OTHER;
}

}
