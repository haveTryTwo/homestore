#ifndef HOME_STORE_MUTEX_LOCK_H_
#define HOME_STORE_MUTEX_LOCK_H_

#include "Mutex.h"

namespace homestore
{

class MutexLock
{
    public:
        MutexLock(Mutex *mu) : mu_(mu)
        {
            mu_->Lock();
        }

        ~MutexLock()
        {
            mu_->UnLock();
        }

    private:
        MutexLock(const MutexLock &);
        MutexLock& operator= (const MutexLock &);

    private:
        Mutex *mu_;
};

}

#endif
