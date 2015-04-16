#include <iostream>

#include <stdint.h>

#include "Common.h"
#include "Status.h"
#include "Message.pb.h"

int TestMessage();

int main(int argc, char *argv[])
{
    TestMessage();

    return 0;
}

int TestMessage()
{
    StoreRequest request;
    request.set_op_type(homestore::H_GET);
    request.set_key("key1");

    std::string tmp;
    request.SerializeToString(&tmp);

    StoreRequest ret_request;
    ret_request.ParseFromString(tmp);
    std::cout << ret_request.op_type() << " : " 
        << ret_request.key() << " : " 
        << ret_request.value() << std::endl;

    StoreResponse reponse;
    reponse.set_ret_type(homestore::H_OK);
    reponse.set_value("good one");
    reponse.SerializeToString(&tmp);
    StoreResponse ret_reponse;
    ret_reponse.ParseFromString(tmp);
    std::cout << ret_reponse.ret_type() << " : "
        << ret_reponse.value() << std::endl;

    return 0;
}
