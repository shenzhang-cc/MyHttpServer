//
// Created by shenzhang-cc on 2020/5/23.
//

#include "RequestData.h"

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string> MimeType::mime;

void MimeType::init() {
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "application/x-ico";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
};

string MimeType::getMime(const string &suffix)
{
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}

RequestData::RequestData():
    events(0),
    error(false),
    now_read_pos(0),
    state(STATE_PARSE_URI),
    h_state(h_start),
    keep_alive(false),
    isAbleRead(true),
    isAbleWrite(false)
{}

RequestData::RequestData(int _epollfd, int _fd, string _path):
        now_read_pos(0),
        state(STATE_PARSE_URI),
        h_state(h_start),
        keep_alive(false),
        path(_path),
        fd(_fd),
        epollfd(_epollfd),
        isAbleRead(true),
        isAbleWrite(false),
        events(0),
        error(false)
{}

RequestData::~RequestData()
{
    close(fd);
}





