#ifndef HOME_STORE_STATUS_H_ 
#define HOME_STORE_STATUS_H_

namespace homestore
{

enum Code
{
    H_OK,
    H_ERR_OTHER = 1,
    H_ERR_NULL_PARAM = 2,
    H_ERR_CONN = 11,
    H_ERR_CONN_FULL,
    H_ERR_SOCKET,
    H_ERR_BIND,
    H_ERR_LISTEN,
    H_ERR_READ,
    H_ERR_OPEN,
    H_ERR_WRITE,
    H_ERR_NOT_EXIST = 31,
    H_ERR_TIMEOUT,
    H_ERR_IO,
};

class Status
{
    public:
        Status(Code c) : code_(c) {}
        Status(const Status &status)
        {
            code_ = status.code_;
        }

        Status& operator= (const Status &status)
        {
            code_ = status.code_;
            return *this;
        }

        ~Status() {}

    private:
        Code code_;
};

}

#endif
