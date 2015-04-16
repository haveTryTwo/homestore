#ifndef HOME_STORE_HOME_SERVER_H_
#define HOME_STORE_HOME_SERVER_H_

#include <string>
#include <deque>

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "Mutex.h"
#include "proto/Common.h"
#include "proto/Status.h"
#include "Db.h"

namespace homestore
{

class HomeServer
{
    public:
        class Worker;

    public:
        HomeServer(std::string dbname, short port, int backlog = H_DEFAULT_BACKOG,
                int worker_num = H_DEFAULT_WORKER_NUM);
        ~HomeServer();

    private:
        HomeServer(const HomeServer &);
        HomeServer& operator= (const HomeServer&);

    private:
        int Init();

    public:
        int Start();

    public:
        const static uint32_t H_CONNS_MAX = 10000;
        const static uint32_t H_DEFAULT_BACKOG = 100;
        const static uint32_t H_DEFAULT_WORKER_NUM = 10;

    public:
        std::string dbname_;
        DB *db_;

        short port_;
        int serv_fd_;
        int backlog_;
        bool running_;
        uint32_t worker_num_;
        std::deque<Worker*> works_;

        struct event *serv_ev_;
        struct event_base *evbase_;

    public:
        class Conn
        {
            public:
                std::string cnt_;
                uint32_t left_size_;
                uint32_t total_size_;

                int cli_fd_;
                short ev_flg_;
                struct event *cli_ev_;
                struct event_base *evbase_;
                enum ConnStatus conn_status_;

                Worker *cur_worker_;
        };

    public:
        class Worker
        { /** {{{ Worker **/
            public:
                Worker(HomeServer *hs);
                ~Worker();

            public:
                // Home Server receive an connection and add this into 
                // Worker accept fds, and notify the worker to deal with it
                int AddAcceptFds(const struct QueueItem &item);
                int MoveAcceptFds();

            private:
                int Init();
                Worker(const Worker &w); 
                Worker& operator= (const Worker &w);

            public:
                bool running_;
                pthread_t w_id_;
                int pipe_fd_[2];
                std::deque<Conn*> conns_;

                struct event_base *evbase_;
                struct event *pipe_ev_;

                HomeServer *hs_;

                Mutex mu_;
                std::deque<QueueItem> accept_fds_;
        }; /** }}} **/
};

}

#endif
