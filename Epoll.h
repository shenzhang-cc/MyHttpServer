//
// Created by shenzhang-cc on 2020/5/23.
//
#pragma once
//#ifndef MYHTTPSERVER_EPOLL_H
//#define MYHTTPSERVER_EPOLL_H

#include <sys/epoll.h>
#include <memory>
#include <vector>
#include "RequestData.h"
#include "Timer.h"

class Epoll {                                                      
public:
    typedef std::shared_ptr<RequestData> SP_ReqData;
private:
    static const int MAXFDS = 1000;
    static epoll_event* events;
    static SP_ReqData fd2req[MAXFDS];
    static int epoll_fd;
    static const std::string PATH;
    static Timer timer_manager;
public:
    static int epoll_init(int maxevents, int listen_num);
    static int epoll_add(int fd, SP_ReqData request, __uint32_t events);
    static int epoll_mod(int fd, SP_ReqData request, __uint32_t events);
    static int epoll_del(int fd, __uint32_t events = (EPOLLIN | EPOLLET | EPOLLONESHOT));
    static void my_epoll_wait(int listen_fd, int max_events, int timeout);
    // 处理服务器监听描述符的的就绪事件的函数
    static void acceptConnection(int listen_fd, int epoll_fd, const std::string path);
    //
    // 将就绪的描述符按类型分发：服务器的监听描述符就派去接收新的连接
    // 客户端的描述符就送到线程池中让工作线程为其服务
    static std::vector<SP_ReqData> getEventsRequest(int listen_fd, int events_num, const std::string path);

    static void add_timer(SP_ReqData request_data, int timeout);
};


//#endif //MYHTTPSERVER_EPOLL_H
