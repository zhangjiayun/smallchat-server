#include "Sockets.h"

SOCKET sockets::createOrDie()
{
#ifdef WIN32
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
    if (sockfd == INVALID_SOCKET)
    {
        LOG("sockets::createOrDie");
    }

    return sockfd;
}

SOCKET sockets::createNonblockingOrDie()
{
#ifdef WIN32
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
    if (sockfd == INVALID_SOCKET)
    {
        LOG("sockets::createNonblockingOrDie");
    }

    setNonBlockAndCloseOnExec(sockfd);
    return sockfd;
}

void sockets::setNonBlockAndCloseOnExec(SOCKET sockfd)
{
#ifdef WIN32
    //将socket设置成非阻塞的
    unsigned long on = 1;
    ::ioctlsocket(sockfd, FIONBIO, &on);
#else
    // non--block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
#endif
}

void sockets::setReuseAddr(SOCKET sockfd, bool on)
{
    int optval = on ? 1 : 0;
#ifdef WIN32
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
#else
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
#endif
    // FIXME CHECK
}

void sockets::setReusePort(SOCKET sockfd, bool on)
{
    //Windows 系统没有 SO_REUSEPORT 选项
#ifndef WIN32
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG("SO_REUSEPORT failed.");
    }
#endif
}

int sockets::bindOrDie(SOCKET sockfd, const sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
    if (ret == SOCKET_ERROR)
    {
        LOG("sockets::bindOrDie");
    }
    return ret;
}

int sockets::listenOrDie(SOCKET sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret == SOCKET_ERROR)
    {
        LOG("sockets::listenOrDie");
        return false;
    }
    return ret;
}

SOCKET sockets::accept(SOCKET sockfd, sockaddr_in* addr)
{
    SOCKET connfd;
    while (1)
    {
        socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);
        memset(addr, 0, addrlen);
#ifdef WIN32
        connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
        setNonBlockAndCloseOnExec(connfd);
#else  
        connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
        if (connfd == SOCKET_ERROR)
        {
#ifdef WIN32
            int savedErrno = ::WSAGetLastError();
            LOG("Socket::accept");
            if (savedErrno != WSAEWOULDBLOCK)
            {
                LOG("unexpected error of ::accept");
                return connfd;
            }
#else
            int savedErrno = errno;
            LOG("Socket::accept");
            switch (savedErrno)
            {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
            {
                LOG("unexpected error of ::accept");
                return connfd;
            }
            break;
            default:
            {
                LOG("unknown error of ::accept");
                return connfd;
            }
            break;
            }

#endif
        }
        break;
    }
    return connfd;
}

int32_t sockets::read(SOCKET sockfd, void* buf, int32_t count)
{
#ifdef WIN32
    return ::recv(sockfd, (char*)buf, count, 0);
#else
    return ::read(sockfd, buf, count);
#endif
}

#ifndef WIN32
ssize_t sockets::readv(SOCKET sockfd, const struct iovec* iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}
#endif

int32_t sockets::write(SOCKET sockfd, const void* buf, int32_t count)
{
#ifdef WIN32
    return ::send(sockfd, (const char*)buf, count, 0);
#else
    return ::write(sockfd, buf, count);
#endif

}

void sockets::close(SOCKET sockfd)
{
#ifdef WIN32   
    if (::closesocket(sockfd) < 0)
#else
    if (::close(sockfd) < 0)
#endif
    {
        LOG("sockets::close");
    }
}

const sockaddr* sockets::sockaddr_cast(const sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>((const void*)(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<struct sockaddr*>((void*)(addr));
}
