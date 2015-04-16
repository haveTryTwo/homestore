#include <iostream>
#include <string>

#include "Util.h"
#include "proto/Log.h"
#include "proto/Common.h"
#include "proto/Message.pb.h"

namespace homestore
{

Code ParseAndSerialize(DB *db, const std::string str, std::string &ret_val)
{
    StoreRequest request;
    request.ParseFromString(str);

    Code ret;
    std::string tmp;
    StoreResponse response;

    switch (request.op_type())
    {
        case H_GET:
            ret = db->Get(request.key(), tmp);
            if (ret == H_OK)
                response.set_value(tmp);
            response.set_ret_type(ret);
            response.SerializeToString(&tmp);
            break;
        case H_PUT:
            ret = db->Put(request.key(), request.value());
            response.set_ret_type(ret);
            response.SerializeToString(&tmp);
            break;
        case H_DEL:
            ret = db->Del(request.key());
            response.set_ret_type(ret);
            response.SerializeToString(&tmp);
            break;
        default:
            break;
    }

    uint32_t len = tmp.size();
    // TODO: deal with little and big endian
    ret_val.assign((const char*)&len, sizeof(uint32_t));
    ret_val.append(tmp);

    return H_OK;
}

}
