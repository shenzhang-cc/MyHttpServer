//
// Created by shenzhang-cc on 2020/5/24.
//

#include "Timer.h"
#include "Epoll.h"

#include <sys/time.h>

TimerNode::TimerNode(SP_ReqData _request_data, int timeout):
    deleted(false),
    request_data(_request_data)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode()
{
    if (request_data)
    {
        Epoll::epoll_del(request_data->getFd());
    }
}

void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}
// 因超时而设定deleted
bool TimerNode::isvalid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    if (temp < expired_time)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

void TimerNode::clearReq()
{
    request_data.reset();
    this->setDeleted();
}

void TimerNode::setDeleted()
{
    deleted = true;
}

bool TimerNode::isDeleted() const
{
    return deleted;
}

size_t TimerNode::getExpTime() const
{
    return expired_time;
}

Timer::Timer() {}
Timer::~Timer() {}

void Timer::addTimer(SP_ReqData request_data, int timeout)
{
    SP_TimerNode newNode(new TimerNode(request_data, timeout));

    {
        MutexLockGuard locker(lock);
        TimerNodeQueue.push(newNode);
    }

    request_data->linkTimer(newNode);
}

void Timer::addTimer(SP_TimerNode timer_node)
{

}

void Timer::handle_expired_event()
{
    MutexLockGuard locker(lock);
    while (!TimerNodeQueue.empty())
    {
        SP_TimerNode ptimer_now = TimerNodeQueue.top();
        if (ptimer_now->isDeleted())
        {
            TimerNodeQueue.pop();
        }
        else if (!ptimer_now->isvalid())
        {
            TimerNodeQueue.top();
        }
        else break;
    }
}