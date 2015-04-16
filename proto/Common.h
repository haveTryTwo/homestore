#ifndef HOME_STORE_COMMON_H_
#define HOME_STORE_COMMON_H_

#include <stdint.h>

namespace homestore
{

struct Common
{
    static const uint32_t H_HEAD_LEN   = 4;
    static const uint32_t H_BUF_LEN   = 4096;
    static const short H_DEFAULT_PORT = 7889;
};

enum ConnStatus
{
    CONN_CMD,
    CONN_WAIT,
    CONN_READ,
    CONN_WRITE,
    CONN_CLOSE,
};

struct QueueItem
{
    int cli_fd_;
    short ev_flg_;
    enum ConnStatus conn_status_;
};

enum OpType
{
    H_GET,
    H_PUT,
    H_DEL,
};

}


#endif
