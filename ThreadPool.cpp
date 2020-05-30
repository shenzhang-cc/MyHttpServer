//
// Created by shenzhang-cc on 2020/5/25.
//

#include <iostream>
#include "ThreadPool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;

std::vector<pthread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::taskQue;
int ThreadPool::queue_size = 0;
int ThreadPool::thread_cnt = 0;
int ThreadPool::task_cnt = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::startedThreads = 0;

int ThreadPool::threadpool_create(int _thread_cnt, int _queue_size) {
    do
    {
        if (_thread_cnt <= 0 || _thread_cnt > MAX_THREADS || _queue_size <= 0 || _queue_size > MAX_QUEUE)
        {
            _thread_cnt = 4;
            _queue_size = 1024;
        }
        queue_size = _queue_size;
        threads.resize(_thread_cnt);
        taskQue.resize(_queue_size);
        // 创建工作线程
        for (int i = 0; i < _thread_cnt; i++)
        {
            if (pthread_create(&threads[i], NULL, threadenter, (void*)(0)) != 0)
            {
                return -1;
            }
            thread_cnt++;
            startedThreads++;
        }
    }while (false);
    return 0;
}

int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun) {
    int next, err = 0;
    // 向任务队列添加任务涉及锁的争用
    if (pthread_mutex_lock(&lock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }
    do
    {
        next = (tail + 1) % queue_size;
        if (task_cnt == queue_size)
        {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        if (shutdown)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        taskQue[tail].fun = fun;
        taskQue[tail].args = args;
        tail = next;
        task_cnt++;
        if (pthread_cond_signal(&notify) != 0)
        {
            err = THREADPOOL_THREAD_FAILURE;
            break;
        }
    }while(false);
    if (pthread_mutex_unlock(&lock) != 0)
    {
        err = THREADPOOL_LOCK_FAILURE;
    }
    return err;
}

int ThreadPool::threadpool_destroy(ShutDownOption shutdown_option) {
    printf("Thread pool destroy !\n");
    int i, err = 0;
    if (pthread_mutex_lock(&lock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }
    do
    {
        if (shutdown)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        shutdown = shutdown_option;
        if ((pthread_cond_broadcast(&notify) != 0) || (pthread_mutex_unlock(&lock) != 0))
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
        for (i = 0; i < thread_cnt; i++)
        {
            if (pthread_join(threads[i], NULL) != 0)
            {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    } while(false);

    if (!err)
    {
        threadpool_free();
    }
    return err;
}

int ThreadPool::threadpool_free() {
    if (startedThreads > 0)
        return -1;
    pthread_mutex_lock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notify);
    return 0;
}

void* ThreadPool::threadenter(void* args){
    while (true)
    {
        ThreadPoolTask task;
        // 从任务队列中提取任务需要争用锁
        pthread_mutex_lock(&lock);
        while ((task_cnt == 0) && (!shutdown))
        {
            pthread_cond_wait(&notify, &lock);
        }
        if ((shutdown == immediate_shutdown) ||
            (shutdown == graceful_shutdown && task_cnt == 0))
        {
            break;
        }
        task.fun = taskQue[head].fun;
        task.args = taskQue[head].args;
        taskQue[head].fun = nullptr;
        taskQue[head].args.reset();
        head = (head + 1) % queue_size;
        task_cnt--;
        pthread_mutex_unlock(&lock);
        task.fun(task.args);
    }
    startedThreads--;
    pthread_mutex_unlock(&lock);
    std::cout << "The thread finished" << std::endl;
    pthread_exit(NULL);
//    return NULL;  // pthread_exit已经退出了线程，执行不到renturn这里
}

void ThreadPool::myHandler(std::shared_ptr<void> req) {
    std::shared_ptr<RequestData> request = std::static_pointer_cast<RequestData>(req);
    if (request->canWrite())
    {

    }
    else if(request->canRead())
    {

    }
    request->handleConnection();
};
