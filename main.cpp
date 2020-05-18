#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>

using namespace std;

static const int LISTENQ = 1024;

int socket_bind_listen(int port) {
    if (port < 1024 || port > 65535)
        return -1;
    int listen_fd = 0;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;
    int optval = 1;
    if ((setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1)
        return -1;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if ((bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1)
        return -1;
    if ((listen(listen_fd, LISTENQ)) == -1)
        return -1;
    return listen_fd;
}

int main() {


    return 0;
}
