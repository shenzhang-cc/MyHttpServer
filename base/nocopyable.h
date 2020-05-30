//
// Created by shenzhang-cc on 2020/5/30.
//

#ifndef MYHTTPSERVER_NOCOPYABLE_H
#define MYHTTPSERVER_NOCOPYABLE_H


class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};


#endif //MYHTTPSERVER_NOCOPYABLE_H
