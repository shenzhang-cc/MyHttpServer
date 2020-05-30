//
// Created by shenzhang-cc on 2020/5/23.
//

#ifndef MYHTTPSERVER_REQUESTDATA_H
#define MYHTTPSERVER_REQUESTDATA_H

#include <zconf.h>
#include <unordered_map>
#include <string>

#include "Timer.h"

using namespace std;

//using namespace cv;

const int STATE_PARSE_URI = 1;
const int STATE_PARSE_HEADERS = 2;
const int STATE_RECV_BODY = 3;
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;

const int MAX_BUFF = 4096;

// 有请求出现但是读不到数据,可能是Request Aborted,
// 或者来自网络的数据没有达到等原因,
// 对这样的请求尝试超过一定的次数就抛弃
const int AGAIN_MAX_TIMES = 200;

const int PARSE_URI_AGAIN = -1;
const int PARSE_URI_ERROR = -2;
const int PARSE_URI_SUCCESS = 0;

const int PARSE_HEADER_AGAIN = -1;
const int PARSE_HEADER_ERROR = -2;
const int PARSE_HEADER_SUCCESS = 0;

const int ANALYSIS_ERROR = -2;
const int ANALYSIS_SUCCESS = 0;

const int METHOD_POST = 1;
const int METHOD_GET = 2;
const int HTTP_10 = 1;
const int HTTP_11 = 2;

const int EPOLL_WAIT_TIME = 500;

class MimeType {
private:
    static void init();
    static unordered_map<string, string> mime;
    MimeType();
    MimeType(const MimeType& m);
public:
    static string getMime(const string &suffix);
private:
    static pthread_once_t once_control;
};

enum HeaderState {
    h_start = 0,
    h_key,
    h_colon,
    h_spaces_after_colon,
    h_value,
    h_CR,
    h_LF,
    h_end_CR,
    h_end_LF
};


class RequestData : public std::enable_shared_from_this<RequestData> {
private:
    string path;
    int fd;
    int epollfd;

    string inBuffer;
    string outBuffer;

    __uint32_t events;
    bool error;

    int method;
    int HTTPversion;
    string fileName;

    int now_read_pos;
    int state;
    int h_state;
    bool isFinish;
    bool keep_alive;
    unordered_map<string, string> headers;
    weak_ptr<TimerNode> timer;

    bool isAbleRead;
    bool isAbleWrite;
private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();

    Mat stitch(Mat &src)
    {
        return src;
    }

public:
    RequestData();
    RequestData(int _epollfd, int _fd, std::string _path);
    ~RequestData();
    void linkTimer();
    void reset();
    void seperateTimer();
    int getFd();
    void setFd();
    void handleRead();
    void handleWrite();
    void handleError();
    void handleConn();
    void disableReadAndWrite();
    void enableRead();
    void enableWrite();
    bool canRead();
    bool canWrite();
};




#endif //MYHTTPSERVER_REQUESTDATA_H
