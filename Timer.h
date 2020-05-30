//
// Created by shenzhang-cc on 2020/5/24.
//

#ifndef MYHTTPSERVER_TIMER_H
#define MYHTTPSERVER_TIMER_H

#include <memory>
#include <queue>
#include <deque>
#include "RequestData.h"
#include "base/mutextlock.h"

//class RequestData;
// 时间戳
class TimerNode {
    typedef std::shared_ptr<RequestData> SP_ReqData;
private:
    bool deleted;
    size_t expired_time;
    SP_ReqData request_data;
public:
    TimerNode(SP_ReqData _request_data, int timeout);
    ~TimerNode();
    void update(int timeout);
    // 判断是否超时
    bool isvalid();
    void clearReq();
    // 若超时，设置delete为true
    void setDeleted();
    // 获取deleted的状态
    bool isDeleted() const;
    //
    size_t getExpTime() const;
};

struct timerCmp {
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
    {
        return a->getExpTime() > b->getExpTime();
    }
};


// 用于给每个已连接描述符盖事件戳和处理超时的连接
class Timer {
    typedef std::shared_ptr<RequestData> SP_ReqData;
    typedef std::shared_ptr<TimerNode> SP_TimerNode;
    std::priority_queue<SP_TimerNode , std::deque<SP_TimerNode>, timerCmp> TimerNodeQueue;
    MutexLock lock;
public:
    Timer();
    ~Timer();
    void addTimer(SP_ReqData request_data, int timeout);
    void addTimer(SP_TimerNode timer_node);
    void handle_expired_event();
};


#endif //MYHTTPSERVER_TIMER_H
