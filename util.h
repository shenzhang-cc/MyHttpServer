//
// Created by shenzhang-cc on 2020/5/23.
//
#pragma once

#include <string>
#include <sys/types.h>

ssize_t Read(int fd, void* buff, size_t n);
ssize_t Read(int fd, std::string &inBuffer);
ssize_t Write(int fd, void* buff, size_t n);
ssize_t Write(int fd, std::string &sbuff);

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);