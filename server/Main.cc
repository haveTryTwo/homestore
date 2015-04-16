#include <iostream>

#include "proto/Log.h"
#include "proto/Common.h"
#include "HomeServer.h"

int test_homeserver();

int main(int argc, char *argv[])
{
    test_homeserver();

    return 0;
}

int test_homeserver()
{
    homestore::set_log_level(homestore::LOG_LVL_INFO);
    short port = homestore::Common::H_DEFAULT_PORT;

    std::string dbname("mydb.db");
    homestore::HomeServer *s = new homestore::HomeServer(dbname, port);
    s->Start();

    return 0;
}
