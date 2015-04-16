#ifndef HOME_STORE_UTIL_H_
#define HOME_STORE_UTIL_H_

#include <string>

#include "Db.h"
#include "proto/Status.h"

namespace homestore
{

Code ParseAndSerialize(DB *db, const std::string str, std::string &ret_val);

}

#endif
