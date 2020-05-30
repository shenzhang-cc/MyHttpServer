//
// Created by shenzhang-cc on 2020/5/23.
//

#include "Epoll.h"
#include "ThreadPool.h"
#include "util.h"
#include <sys/socket.h>
#include <string.h>

using namespace std;
// 用于用来从内核得到就绪事件的集合
epoll_event* Epoll::events;
// fd2req:fd to req,用一个数组作为哈希表，key为下标fd，
// value为一个指向RequestData对象的shared_ptr
// 用于存放每一个客户端的请求
Epoll::SP_ReqData Epoll::fd2req[MAXFDS];
int Epoll::epoll_fd = 0;
const std::string Epoll::PATH = "/";
Timer Epoll::timer_manager;

int Epoll::epoll_init(int maxevents, int listen_num)
{
    // +1是因为epoll句柄本身就会占用一个fd值
    epoll_fd = epoll_create(listen_num + 1);
    if (epoll_fd == -1)
    {
        return -1;
    }
    events = new epoll_event[maxevents];
    return 0;
}

int Epoll::epoll_add(int fd, SP_ReqData request, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    fd2req[fd] = request;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        return -1;
    }
    return 0;
}

int Epoll::epoll_mod(int fd, SP_ReqData request, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    fd2req[fd] = request;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("epoll_mod error");
        // 若request是唯一指向其对象的shared_ptr，则reset会释放此对象
        request.reset();
        return -1;
    }
    return 0;
}

int Epoll::epoll_del(int fd, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
        return -1;
    }
    // 删除这个fd在哈希表中的记录
    fd2req[fd].reset();
    return 0;
}

void Epoll::my_epoll_wait(int listen_fd, int max_events, int timeout)
{
    // max_events 告诉内核这个events数组有多大
    // event_count 为活跃的io事件的总数
    int event_count = epoll_wait(epoll_fd, events, max_events, timeout);
    if (event_count < 0)
        perror("epoll wait error");
    std::vector<SP_ReqData> req_data = getEventsRequest(listen_fd, event_count, PATH);
    if (req_data.size() > 0)
    {
        for (auto& req : req_data)
        {
            // 这里失败的原因不是请求本身，本身可能出现错误的请求已经在分发处理的时候排除过了。
            // 因此这里出错是线程池的问题，可能是任务队列满了或者线程池已经关闭了。
            // 则直接跳出循环，不再添加任务。
            if (ThreadPool::threadpool_add() < 0)
            {
                break;
            }
        }
    }
    // 本论监听的事件处理完毕，检查计时器队列
    // 剔除超时的请求
    timer_manager.handle_expired_event();
}

#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

void Epoll::acceptConnection(int listen_fd, int epoll_fd, const std::string path)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    //  memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while ((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        cout << inet_ntoa(client_addr.sin_addr) << endl;
        cout << ntohs(client_addr.sin_port) << endl;
        /*
        // TCP的保活机制默认是关闭的
        int optval = 0;
        socklen_t len_optval = 4;
        getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
        cout << "optval ==" << optval << endl;
        */
        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }

        if (setSocketNonBlocking(accept_fd) < 0)
        {
            perror("Set non block failed!");
            return;
        }

        SP_ReqData request((new RequestData);
        __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
        Epoll::epoll_add(accept_fd, request, _epo_event);
        // 接收连接时刻起，就为该已连接套接字增加时间戳
        timer_manager.addTimer();
    }
}

vector<Epoll::SP_ReqData> Epoll::getEventsRequest(int listen_fd, int events_num, const std::string path)
{
    vector<SP_ReqData> req_data;
    for (int i = 0; i < events_num; i++)
    {
        int fd = events[i].data.fd;
        if (fd == listen_fd)
        {
            acceptConnection();
        }
        else if (fd < 3) // 012为标准输入输出和错误流
        {
            break;
        }
        else
        {
            // 文件描述符 发生错误 或 被挂断
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
            {
                perror("error event\n");
                if (fd2req[fd])
                {
                    // 不再对这个fd指示的连接计时
                    fd2req[fd].seperateTimer();
                }
                fd2req[fd].reset();  //删除哈希表中的记录
                continue;
            }
            // 若是已连接描述符，则送到线程池中让工作线程为其提供服务
            // 加入线程池之前将Timer和request分离
            // 这里只是将所有已连接描述符放到一个vector里，添加到线程池的动作由主调函数完成
            // 其实好像没必要（小声bb）
            SP_ReqData cur_req = fd2req[fd];
            if (cur_req)
            {
                if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
                    cur_req->enableRead();
                else
                    // 因为在acceptConnection的时候，将新的已连接描述符注册进epoll时，
                    // 注册了EPOLLIN 和 EPOLLET
                    // 如果EPOLLET发生了，但又不是EPOLLIN，则只能是可写了
                    cur_req->enableWrite();
                cur_req->seperateTimer();
                req_data.push_back(cur_req);
                // 因为注册监听事件时设置了EPOLLONESHOT
                // 所以这一次处理了请求之后，就删掉在哈希表中的记录
                // 这个fd对应的连接如果需要继续监听时，会在处理请求完成之后再次注册到epoll
                fd2req[fd].reset();
            }
            else
            {
                perror("invalid req");
            }
        }
    }
    return req_data;
}
