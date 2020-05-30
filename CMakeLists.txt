cmake_minimum_required(VERSION 3.15)
project(MyHttpServer)

set(CMAKE_CXX_STANDARD 11)

add_executable(MyHttpServer main.cpp util.cpp util.h Epoll.cpp Epoll.h RequestData.cpp RequestData.h Timer.cpp Timer.h ThreadPool.cpp ThreadPool.h base/mutextlock.h base/nocopyable.h base/condition.h)