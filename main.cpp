#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include "util.h"

#include "Epoll.h"
#include "ThreadPool.h"

using namespace std;

static const int LISTENQ = 1024;
static const int MAXEVENTS =  5000;

const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;

const int PORT = 8888;

int socket_bind_listen(int port) {
    if (port < 1024 || port > 65535)
        return -1;
    int listen_fd = 0;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        close(listen_fd);
        return -1;
    }

    // 将listen_fd设置为允许重用本地端口
    int optval = 1;
    if ((setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1)
        return -1;

    // 将listen_fd与一个本地协议地址绑定起来
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if ((bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1)
        return -1;

    if ((listen(listen_fd, LISTENQ)) == -1) // LISTENQ指明在该套接字上可等待的最大连接数。
        return -1;
    return listen_fd;
}


int main() {
    handle_for_sigpipe();
    if (Epoll::epoll_init(MAXEVENTS, LISTENQ) < 0)
    {
        perror("Epoll init error");
        return 1;
    }

    if (ThreadPool::threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE) != 0)
    {
        perror("Threadpool create failed");
        return 1;
    }
    int listen_fd = socket_bind_listen(PORT);
    if (listen_fd < 0)
    {
        perror("sock bind failed");
        return 1;
    }
    if (setSocketNonBlocking(listen_fd) < 0)
    {
        perror("set socket non block failed");
        return 1;
    }
    shared_ptr<RequestData> request (new RequestData());
    request->setfd();

    if (Epoll::epoll_add(listen_fd, request, EPOLLIN | EPOLLET) < 0)
    {
        perror("epoll add failed");
        return 1;
    }
    while (true)
    {
        Epoll::my_epoll_wait(listen_fd, MAXEVENTS, -1);
    }
    return 0;
}
