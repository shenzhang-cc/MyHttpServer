//
// Created by shenzhang-cc on 2020/5/25.
//

#ifndef MYHTTPSERVER_THREADPOOL_H
#define MYHTTPSERVER_THREADPOOL_H

#include "RequestData.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown = 2
} ShutDownOption;

struct ThreadPoolTask {
    std::function<void(std::shared_ptr<void>)> fun;
    std::shared_ptr<void> args;
};


class ThreadPool {
private:
    static pthread_mutex_t lock;
    static pthread_cond_t notify;

    static std::vector<pthread_t> threads;
    static std::vector<ThreadPoolTask> taskQue;
    static int queue_size;
    static int thread_cnt;
    static int task_cnt;
    static int head;  // 任务队列中首元素的下标
    static int tail; //
    static int shutdown;
    static int startedThreads;
    static void myHandler(std::shared_ptr<void> req);
public:
    static int threadpool_create(int _thread_cnt, int _queue_size);
    static int threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun = myHandler);
    static int threadpool_destroy(ShutDownOption shutdown_option);
    static int threadpool_free();
    static void* threadenter(void* args);
};


#endif //MYHTTPSERVER_THREADPOOL_H
