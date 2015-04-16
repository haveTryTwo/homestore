#include <iostream>
#include <string>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "HomeClient.h"
#include "proto/Log.h"
#include "proto/Common.h"

namespace homestore
{

int HStore::Init()
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);

    int ret = inet_pton(AF_INET, host_.c_str(), &(serv_addr.sin_addr));
    assert(ret == 1);

    cli_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd_ == -1)
    {
        LOG_ERR("Failed to create socket! errno:%d", errno);
        return H_ERR_SOCKET;
    }

    ret = connect(cli_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1)
    {
        LOG_ERR("Failed to connect to host:%s, port:%d! errno:%d",
                host_.c_str(), port_, errno);
        return H_ERR_CONN;
    }

    return H_OK;
}

Code HStore::Get(const std::string &key, std::string &value)
{
    StoreRequest request;
    StoreResponse response;

    request.set_op_type(H_GET);
    request.set_key(key);

    Code ret = SendAndRecv(request, response);
    if (ret != H_OK)
    {
        LOG_ERR("Failed to exec SendAndRecv! ret:%d", ret);
        return ret;
    }

    if (response.ret_type() == H_OK)
        value = response.value();

    return (Code)response.ret_type();
}

Code HStore::Put(const std::string &key, const std::string &value)
{
    StoreRequest request;
    StoreResponse response;

    request.set_op_type(H_PUT);
    request.set_key(key);
    request.set_value(value);

    Code ret = SendAndRecv(request, response);
    if (ret != H_OK)
    {
        LOG_ERR("Failed to exec SendAndRecv! ret:%d", ret);
        return ret;
    }

    return (Code)response.ret_type();
}

Code HStore::Del(const std::string &key)
{
    StoreRequest request;
    StoreResponse response;

    request.set_op_type(H_DEL);
    request.set_key(key);

    Code ret = SendAndRecv(request, response);
    if (ret != H_OK)
    {
        LOG_ERR("Failed to exec SendAndRecv! ret:%d", ret);
        return ret;
    }

    return (Code)response.ret_type();
}

Code HStore::SendAndRecv(const StoreRequest &req, StoreResponse &res)
{
    std::string str, tmp;
    req.SerializeToString(&tmp);

    uint32_t buf_len = tmp.size();
    // TODO: consider to little and big endian
    str.append((const char*)&buf_len, sizeof(uint32_t));
    str.append(tmp);

    // Send the request
    Code ret = SendRequest(str);
    if (ret != H_OK)
    {
        LOG_ERR("Faield to send request! ret:%d", ret);
        return ret;
    }

    // Get the response
    ret = RecvResponse(str);
    if (ret != H_OK)
    {
        LOG_ERR("Failed to recv response! ret:%d", ret);
        return ret;
    }

    // Parse from the response
    res.ParseFromString(str);

    return ret;
}

Code HStore::RecvResponse(std::string &str)
{
    char head_buf[Common::H_HEAD_LEN];
    char read_buf[Common::H_BUF_LEN];
    uint32_t left_size = 0;
    int ret = 0;

    // read head buffer
    left_size = Common::H_HEAD_LEN;
    while (left_size > 0)
    {
        ret = read(cli_fd_, head_buf, left_size);
        if (ret == -1 || ret == 0)
        {
            LOG_ERR("Failed to read! errno:%d", errno);
            return H_ERR_READ;
        }

        left_size = left_size - ret;
    }

    // read content buffer
    str.clear();
    // TODO: to consider little and big endian
    left_size = *(uint32_t*)head_buf;
    while (left_size > 0)
    {
        int lsize = left_size <= Common::H_BUF_LEN ? 
            left_size : Common::H_BUF_LEN;

        ret = read(cli_fd_, read_buf, lsize);
        if (ret == -1 || ret == 0)
        {
            LOG_ERR("Failed to read! errno:%d", errno);
            return H_ERR_READ;
        }

        str.append(read_buf, ret);
        left_size = left_size - ret;
    }

    return H_OK;
}

Code HStore::SendRequest(const std::string &str)
{
    uint32_t left_size = str.size();
    while (left_size > 0)
    {
        int ret = write(cli_fd_, str.data() + str.size() - left_size, str.size());
        if (ret == -1)
        {
            LOG_ERR("Failed to write! errno:%d", errno);
            return H_ERR_WRITE;
        }
        left_size = str.size() - ret;
    }

    return H_OK;
}

}

int main(int argc, char *argv[])
{
    short port = homestore::Common::H_DEFAULT_PORT;
    homestore::HStore *hs = new homestore::HStore("127.0.0.1", port);

    std::string value;
    homestore::Code code = hs->Get("key1", value);
    std::cout << "Return Code of Get:" << code << std::endl;
    std::cout << value << std::endl;

    code = hs->Put("key1", "zhangsan");
    std::cout << "Return Code of Put:" << code << std::endl;

    code = hs->Del("key");
    std::cout << "Return Code of Del:" << code << std::endl;

    std::string str("key");
    std::string key;
    char buf[24];
    /*
    for (uint32_t i = 0; i < 1000; ++i)
    {
        snprintf(buf, sizeof(buf), "-%d", i);
        key.assign(str);
        key.append(buf, strlen(buf));
        value = key;
        code = hs->Put(key, value);
        assert(code == homestore::H_OK);
    }
    */

    for (uint32_t i = 0; i < 1000; ++i)
    {
        snprintf(buf, sizeof(buf), "-%d", i);
        key.assign(str);
        key.append(buf, strlen(buf));
        value = key;
        code = hs->Get(key, value);
        assert(code == homestore::H_OK);
        std::cout << key << " : " << value << std::endl;
    }
    return 0;
}
