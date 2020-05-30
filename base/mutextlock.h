//
// Created by shenzhang-cc on 2020/5/30.
//

#ifndef MYHTTPSERVER_MUTEXTLOCK_H
#define MYHTTPSERVER_MUTEXTLOCK_H
#include "nocopyable.h"
#include <pthread.h>

class MutexLock: noncopyable {
public:
    MutexLock()
    {
        pthread_mutex_init(&mutex, NULL);
    }
    ~MutexLock()
    {
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_t* get()
    {
        return &mutex;
    }
private:
    pthread_mutex_t mutex;
    // 友元
    friend class Condition;
};

class MutexLockGuard: noncopyable {
public:
    explicit MutexLockGuard(MutexLock& _mutex):
    mutex(_mutex)
    {
        mutex.lock();
    }
    ~MutexLockGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock &mutex;
};


#endif //MYHTTPSERVER_MUTEXTLOCK_H
