#ifndef HOME_STORE_MUTEX_H_
#define HOME_STORE_MUTEX_H_

#include <pthread.h>

namespace homestore
{

class Mutex
{
    public:
        Mutex() 
        {
            pthread_mutex_init(&mutex_, NULL);
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&mutex_);
        }

    public:
        void Lock()
        {
            pthread_mutex_lock(&mutex_);
        }

        void UnLock()
        {
            pthread_mutex_unlock(&mutex_);
        }

    private:
        Mutex(const Mutex &);
        Mutex& operator= (const Mutex &);

    private:
        pthread_mutex_t mutex_;
};

}

#endif
