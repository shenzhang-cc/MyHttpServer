//
// Created by shenzhang-cc on 2020/5/30.
//

#ifndef MYHTTPSERVER_CONDITION_H
#define MYHTTPSERVER_CONDITION_H

#include "nocopyable.h"
#include "mutextlock.h"

class Condition: noncopyable {
public:
    explicit Condition(MutexLock& _mutex):
        mutex(_mutex)
    {
        pthread_cond_init(&cond, NULL);
    }
    ~Condition()
    {
        pthread_cond_destroy(&cond);
    }
    void wait()
    {
        pthread_cond_wait(&cond, mutex.get());
    }
    void notify()
    {
        pthread_cond_signal(&cond);
    }
    void notifyAll()
    {
        pthread_cond_broadcast(&cond);
    }
private:
    MutexLock &mutex;
    pthread_cond_t cond;
};

#endif //MYHTTPSERVER_CONDITION_H
