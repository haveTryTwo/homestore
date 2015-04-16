#ifndef HOME_STORE_HOME_CLIENT_H_
#define HOME_STORE_HOME_CLIENT_H_

#include "proto/Status.h"
#include "proto/Common.h"
#include "proto/Message.pb.h"

namespace homestore 
{

class HStore
{
    public:
        HStore(std::string host, short port) : 
            host_(host), port_(port), cli_fd_(-1)
        { Init(); }

        ~HStore();

    private:
        int Init();
        HStore(const HStore &hs);
        HStore& operator= (const HStore &hs);

    public:
        Code Get(const std::string &key, std::string &value);
        Code Put(const std::string &key, const std::string &value);
        Code Del(const std::string &key);

    private:
        Code SendRequest(const std::string &str);
        Code RecvResponse(std::string &str);
        Code SendAndRecv(const StoreRequest &req, StoreResponse &res); 

    private:
        std::string host_;
        short port_;
        int cli_fd_;
};

}

#endif
