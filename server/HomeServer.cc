#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <event.h>

#include "Util.h"
#include "proto/Log.h"
#include "MutexLock.h"
#include "HomeServer.h"
#include "LevelDbImpl.h"

namespace homestore
{

void* WorkFunc(void *p);
static void OnCliAction(evutil_socket_t fd, short event, void *argv);
static void OnPipeAction(evutil_socket_t pfd, short event, void *argv);
static void OnAcceptAction(evutil_socket_t fd, short event, void *argv);

HomeServer::HomeServer(std::string dbname, short port, int backlog, int worker_num) :
        dbname_(dbname),
        port_(port), backlog_(backlog),
        running_(true), worker_num_(worker_num)

{
    Init();
}

HomeServer::~HomeServer()
{
    running_ = false;
    std::deque<Worker *>::iterator iter = works_.begin();
    while (iter != works_.end())
    {
        Worker *w = *iter;
        delete w;
        works_.pop_front();
        iter = works_.begin();
    }

    // TODO: struct event

    if (db_ != NULL) 
    {
        delete db_;
        db_ = NULL;
    }

    close(serv_fd_);
    serv_fd_ = -1;
}

int HomeServer::Init()
{
    db_ = new LevelDB(dbname_);

    serv_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd_ == -1)
    {
        LOG_ERR("Failed to open socket! errno:%d", errno);
        return H_ERR_SOCKET;
    }

    int opt = 1;
    setsockopt(serv_fd_, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));
    int flags = fcntl(serv_fd_, F_GETFL);
    fcntl(serv_fd_, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(serv_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1)
    {
        LOG_ERR("Failed to bind! errno:%d", errno);
        return H_ERR_BIND;
    }

    ret = listen(serv_fd_, backlog_);
    if (ret == -1)
    {
        LOG_ERR("Failed to listen! errno:%d", errno);
        return H_ERR_LISTEN;
    }

    for (uint32_t i = 0; i < worker_num_; ++i)
    {
        Worker *w = new Worker(this);
        works_.push_back(w);
    }

    srandom(time(NULL));
    return H_OK;
}

int HomeServer::Start()
{
    evbase_ = event_base_new();
    serv_ev_ = new event;

    event_set(serv_ev_, serv_fd_, EV_READ|EV_PERSIST,
            OnAcceptAction, (void*)this);
    event_base_set(evbase_, serv_ev_);
    event_add(serv_ev_, NULL);

    event_base_dispatch(evbase_);

    return H_OK;
}

static void OnAcceptAction(evutil_socket_t fd, short event, void *argv)
{
    HomeServer *hs = (HomeServer*)argv;

    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);

    int cli_fd = accept(fd, (struct sockaddr*)&cli_addr, &cli_addr_len);

    if (cli_fd == -1)
    { 
        if (errno != EINTR)
            LOG_ERR("Failed to accept! errno:%d\n", errno);
    }
    else 
    {
        int opt = 1;
        setsockopt(cli_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));
        int flags = fcntl(cli_fd, F_GETFL);
        fcntl(cli_fd, F_SETFL, flags | O_NONBLOCK);

        assert(hs->works_.size() != 0);
        int n = rand() % hs->works_.size();
        HomeServer::Worker *w = hs->works_[n];

        QueueItem item;
        item.cli_fd_ = cli_fd;
        item.ev_flg_ = EV_READ|EV_PERSIST;
        item.conn_status_ = CONN_CMD;
        w->AddAcceptFds(item);
    }
}

HomeServer::Worker::Worker(HomeServer *hs) : 
    running_(true), hs_(hs)
{
    Init();
}

HomeServer::Worker::~Worker()
{
    running_ = false;
    pthread_join(w_id_, NULL);
    close(pipe_fd_[0]);
    close(pipe_fd_[1]);
    accept_fds_.clear();
    //TODO: free queue conns_
}

int HomeServer::Worker::Init()
{
    int ret = pipe(pipe_fd_);
    assert(ret == 0);

    ret = pthread_create(&w_id_, NULL, WorkFunc, this);
    assert(ret == 0);

    return H_OK;
}

int HomeServer::Worker::AddAcceptFds(const struct QueueItem &item)
{
    MutexLock l(&mu_);
    accept_fds_.push_back(item);

    char buf[1] = {'1'};
    int ret = write(pipe_fd_[1], buf, 1);
    if (ret < 0)
    {
        LOG_ERR("Failed to write pipe fd! errno:%d", errno);
        return H_ERR_WRITE;
    }

    return H_OK;
}

int HomeServer::Worker::MoveAcceptFds()
{
    MutexLock l(&mu_);
    std::deque<QueueItem>::iterator it = accept_fds_.begin();
    for (; it != accept_fds_.end();)
    {
        Conn *conn = new Conn;
        conn->total_size_ = 0;
        conn->left_size_ = 0;
        conn->evbase_ = evbase_;
        conn->cli_ev_ = new event;
        conn->ev_flg_ = it->ev_flg_;
        conn->cli_fd_ = it->cli_fd_;
        conn->conn_status_ = it->conn_status_;
        conn->cur_worker_ = this;

        event_set(conn->cli_ev_, conn->cli_fd_, conn->ev_flg_, 
                    OnCliAction, (void*)conn);
        event_base_set(conn->evbase_, conn->cli_ev_);
        event_add(conn->cli_ev_, NULL);

        conns_.push_back(conn);

        accept_fds_.pop_front();
        it = accept_fds_.begin();
    }

    return H_OK;
}

static void CloseConn(HomeServer::Conn *conn)
{
    HomeServer::Worker *w = conn->cur_worker_;
    event_del(conn->cli_ev_);
    delete conn->cli_ev_;
    close(conn->cli_fd_);

    MutexLock l(&(w->mu_));
    std::deque<HomeServer::Conn*>::iterator it = w->conns_.begin();
    while (it != w->conns_.end())
    {
        if (*it == conn)
        {
            w->conns_.erase(it);
            delete conn;
            break;
        }
    }
}

static void OnCliAction(evutil_socket_t fd, short event, void *argv)
{
    HomeServer::Conn *conn = (HomeServer::Conn*)argv;

    assert(fd == conn->cli_fd_);
    int ret = 0;
    uint32_t lsize = 0;
    char cmd_buf[Common::H_HEAD_LEN];
    char read_buf[Common::H_BUF_LEN];

    LOG_INFO("fd:%d received! status:%d", fd, conn->conn_status_);
    switch (conn->conn_status_)
    {
        case CONN_CMD:
            /** {{{ **/
            conn->cnt_.clear();
            ret = read(conn->cli_fd_, cmd_buf, sizeof(cmd_buf));
            
            // if connect is closed
            if ((ret == -1 && errno == ECONNRESET) || ret == 0)
            {
                CloseConn(conn);
                break;
            }

            // if the length is less than length of command
            if ((uint32_t)ret < sizeof(cmd_buf))
            {
                conn->total_size_ = sizeof(cmd_buf);
                conn->cnt_.append(cmd_buf, ret);
                conn->left_size_ = sizeof(cmd_buf) - ret;
                conn->conn_status_ = CONN_WAIT;
                break;
            }

            // if the length is enough
            // TODO: consider big and little endian
            conn->total_size_ = *(uint32_t*)cmd_buf;
            conn->left_size_ = conn->total_size_;
            conn->conn_status_ = CONN_READ;
            LOG_INFO("Cmd: total size:%u", conn->total_size_);
            break; /** }}} **/
        case CONN_WAIT:
            /** {{{ **/
            ret = read(conn->cli_fd_, cmd_buf, conn->left_size_);
            
            // if connect is closed
            if ((ret == -1 && errno == ECONNRESET) || ret == 0)
            {
                CloseConn(conn);
                break;
            }

            conn->cnt_.append(cmd_buf, ret);
            conn->left_size_ = conn->left_size_ - ret;

            if (conn->left_size_ == 0)
            {
                // TODO: consider big and little endian
                conn->total_size_ = *(uint32_t*)conn->cnt_.data();
                conn->left_size_ = conn->total_size_;
                conn->conn_status_ = CONN_READ;
                conn->cnt_.clear();
                break;
            }
            break; /** }}} **/
        case CONN_READ:
            /** {{{ **/
            memset(read_buf, 0, sizeof(read_buf));
            lsize = conn->left_size_ <= sizeof(read_buf) ? 
                            conn->left_size_ : sizeof(read_buf);
                        
            ret = read(conn->cli_fd_, read_buf, lsize);

            // if connect is closed
            if ((ret == -1 && errno == ECONNRESET) || ret == 0)
            {
                CloseConn(conn);
                break;
            }

            conn->cnt_.append(read_buf, ret);
            conn->left_size_ = conn->left_size_ - ret;

            LOG_INFO("Read total size:%u, left size:%u", conn->total_size_, conn->left_size_);
            if (conn->left_size_ == 0)
            {
                // TODO: do something get or put and then return
                Code r = ParseAndSerialize(conn->cur_worker_->hs_->db_, conn->cnt_, conn->cnt_);
                if (r != H_OK)
                    LOG_ERR("Failed to Parse and Serialize! ret:%d", r);
                conn->total_size_ = conn->cnt_.size();
                conn->left_size_ = conn->total_size_;
                conn->conn_status_ = CONN_WRITE;

                event_del(conn->cli_ev_);
                event_set(conn->cli_ev_, conn->cli_fd_, EV_WRITE|EV_PERSIST, 
                        OnCliAction, (void*)conn);
                event_base_set(conn->evbase_, conn->cli_ev_);
                event_add(conn->cli_ev_, NULL);
                break;
            }
            break; /** }}} **/
        case CONN_WRITE:
            /** {{{ **/
            if (conn->left_size_ == 0)
            {
                conn->cnt_.clear();
                conn->total_size_ = 0;
                conn->left_size_ = 0;
                conn->conn_status_ = CONN_CMD;
                event_del(conn->cli_ev_);
                event_set(conn->cli_ev_, conn->cli_fd_, EV_READ|EV_PERSIST, 
                        OnCliAction, (void*)conn);
                event_base_set(conn->evbase_, conn->cli_ev_);
                event_add(conn->cli_ev_, NULL);
                break;
            }

            ret = write(conn->cli_fd_, conn->cnt_.data() + conn->cnt_.size() - conn->left_size_,
                    conn->left_size_);

            // if connect is closed
            if ((ret == -1 && errno == ECONNRESET) || ret == 0)
            {
                CloseConn(conn);
                break;
            }

            conn->left_size_ = conn->left_size_ - ret;
            LOG_INFO("write total size:%u, left size:%u", conn->total_size_, conn->left_size_);
            break; /** }}} **/
        case CONN_CLOSE:
            break;
        default:
            break;
    }
}

static void OnPipeAction(evutil_socket_t pfd, short event, void *argv)
{
    HomeServer::Worker *w = (HomeServer::Worker*)argv;

    char buf[1];
    int ret = read(w->pipe_fd_[0], buf, 1);
    if (ret <= 0)
    {
        LOG_ERR("Failed to read from pipe! errno:%d", errno);
    }

    w->MoveAcceptFds();
}

void* WorkFunc(void *p)
{
    HomeServer::Worker *w = (HomeServer::Worker*)p;

    w->evbase_ = event_base_new();
    assert(w->evbase_ != NULL);

    w->pipe_ev_ = new event;
    event_set(w->pipe_ev_, w->pipe_fd_[0], EV_READ|EV_PERSIST, OnPipeAction, (void*)w);
    event_base_set(w->evbase_, w->pipe_ev_);

    // add the pipe
    event_add(w->pipe_ev_, NULL);

    event_base_dispatch(w->evbase_);

    pthread_exit(NULL);
}

} // homestore
