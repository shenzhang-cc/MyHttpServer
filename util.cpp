//
// Created by shenzhang-cc on 2020/5/23.
//

#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

const int MAX_BUFF = 4096;

ssize_t Read(int fd, void* buff, size_t n) {
    // 剩余的未读的数据量
    size_t nleft = n;  // typedef unsigned int size_t
    // 单次读取到的字节数
    ssize_t nread = 0;  // signed size_t
    // 读取的总字节数
    ssize_t readSum = 0;
    char* ptr = (char*) buff;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN)
                return readSum;
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
            break;
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

ssize_t Read(int fd, std::string &inBuffer) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
                return readSum;
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            break;
        }
        readSum += nread;
        // string 有一个构造函数是：string (const char* s, size_t n)
        inBuffer += std::string(buff, buff + nread);
    }
    return readSum;
}

ssize_t Write(int fd, void* buff, size_t n) {
    size_t nleft = n;
    ssize_t nwrite = 0;
    ssize_t writeSum = 0;
    char* ptr = (char*) buff;
    while (nleft > 0)
    {
        if ((nwrite = write(fd, buff, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                nwrite = 0;
            }
            else if (errno == EAGAIN)
                return writeSum;
            else return -1;
        }
        writeSum += nwrite;
        nleft -= nwrite;
        ptr += nwrite;
    }
    return writeSum;
}

ssize_t Write(int fd, std::string &sbuff) {
    size_t nleft = sbuff.size();
    ssize_t nwrite = 0;
    ssize_t writeSum = 0;
    const char* ptr = sbuff.c_str();
    while (nleft > 0)
    {
        if (nwrite == write(fd, ptr, nleft) < 0)
        {
            if (errno == EINTR)
                nwrite = 0;
            else if (errno == EAGAIN)
                break;
            else return -1;
        }
        writeSum += nwrite;
        nleft -= nwrite;
        ptr += nwrite;
    }
    if (writeSum == sbuff.size())
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}

void handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL))
        return;
}

int setSocketNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        return -1;
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}