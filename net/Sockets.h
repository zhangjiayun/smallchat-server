#pragma once
#include "platform.h"
//class Socket
//{
//public:
//    explicit Socket(SOCKET sockfd) : m_sockfd(sockfd) { }
//    ~Socket();
//
//    SOCKET fd() const { return m_sockfd; }
//
//    void listen();
//
//    int accept();
//
//private:
//    const SOCKET m_sockfd;
//};

namespace sockets
{
    SOCKET createOrDie();
    SOCKET createNonblockingOrDie();

    void setNonBlockAndCloseOnExec(SOCKET sockfd);

    void setReuseAddr(SOCKET sockfd, bool on);
    void setReusePort(SOCKET sockfd, bool on);

    //SOCKET connect(SOCKET sockfd, const struct sockaddr_in& addr);
    int bindOrDie(SOCKET sockfd, const struct sockaddr_in& addr);
    int listenOrDie(SOCKET sockfd);
    SOCKET accept(SOCKET sockfd, struct sockaddr_in* addr);
    int32_t read(SOCKET sockfd, void* buf, int32_t count);
#ifndef WIN32
    ssize_t readv(SOCKET sockfd, const struct iovec* iov, int iovcnt);
#endif
    int32_t write(SOCKET sockfd, const void* buf, int32_t count);
    void close(SOCKET sockfd);

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
}
