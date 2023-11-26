#include "TcpServer.h"

TcpServer::TcpServer()
    : m_numChannelFd(0),
    m_maxChannelFd(-1),
    m_bInit(false)
{
    if (Init())
    {
        Channel* acceptChannel = new Channel(m_serverSock);
        acceptChannel->setReadCallback(std::bind(&TcpServer::acceptRead, this));
        //将监听socket 放入 m_fdChannelMap 统一管理
        m_fdChannelMap[m_serverSock] = acceptChannel;
        m_usefulFds.insert(m_serverSock);
        m_maxChannelFd = m_serverSock;
    }
}

TcpServer::~TcpServer()
{
    UnInit();

    for (auto it : m_fdChannelMap)
    {
        if (it.second == NULL)
            continue;

        delete it.second;
        it.second = NULL;
    }
}

void TcpServer::run()
{
    printf("The charserver has started, and now you can connect to it!\n");
    while (1)
    {
        fd_set readfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&readfds);

        //start 去掉已失效的fd 
        if (m_clearFds.size() > 0)
        {
            for (const auto& it : m_clearFds)
            {
                if (m_usefulFds.count(it) > 0)
                    m_usefulFds.erase(it);
            }
            m_clearFds.clear();
        }

        for (const auto& fd : m_usefulFds)
        {
            FD_SET(fd, &readfds);
        }

        tv.tv_sec = 1; // 1 sec timeout
        tv.tv_usec = 0;
        SOCKET maxFd = *(m_usefulFds.rbegin());
        retval = select(*(m_usefulFds.rbegin()) + 1, &readfds, NULL, NULL, &tv);
        //end

        if (retval == -1)
        {
            perror("select() error");
            exit(1);
        }
        else if (retval)
        {
            for (const auto& fd : m_usefulFds)
            {

                if (m_fdChannelMap[fd] == NULL)
                    continue;

                if (FD_ISSET(fd, &readfds))
                {
                    m_fdChannelMap[fd]->onRead();
                }
            }
        }

    }
}

bool TcpServer::Init()
{
    InitSocket();
    //创建监听socket
    m_serverSock = sockets::createNonblockingOrDie();
    if (m_serverSock == INVALID_SOCKET)
    {
        LOG("Failed to createNonblockingOrDie");
        return false;
    }
    sockets::setReuseAddr(m_serverSock, true);
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (sockets::bindOrDie(m_serverSock, sa) == -1 ||
        sockets::listenOrDie(m_serverSock) == -1)
    {
        sockets::close(m_serverSock);
        return false;
    }

    return true;
}

void TcpServer::InitSocket()
{
#ifdef WIN32
    if (m_bInit)
        return;

    //初始化socket库
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int err = ::WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        UnInit();
        return;
    }

    m_bInit = true;
#endif
}

void TcpServer::UnInit()
{
#ifdef WIN32
    if (m_bInit)
        ::WSACleanup();
#endif
}

void TcpServer::acceptRead()
{
    struct sockaddr_in sa;
    SOCKET clientFd = sockets::accept(m_serverSock, &sa);
    if (clientFd >= 0)
    {
        Channel* clientChannel = new Channel(clientFd);
        char nick[32];
#ifdef WIN32
        int nicklen = sprintf_s(nick, sizeof(nick), "user:%d", clientFd);
#else
        int nicklen = snprintf(nick, sizeof(nick), "user:%d", clientFd);
#endif
        clientChannel->setNick(nick);
        clientChannel->setReadCallback(std::bind(&TcpServer::generalRead, this, std::placeholders::_1));

        if (clientFd > m_maxChannelFd)
            m_maxChannelFd = clientFd;
        m_fdChannelMap[clientFd] = clientChannel;
        m_usefulFds.insert(clientFd);

        /* Send a welcome message. */
        std::string welcome_msg = "Welcome to Simple Chat! Use /nick <nick> to set your nick.\n";
        int iret = sockets::write(clientFd, (void*)(welcome_msg.c_str()), welcome_msg.size());
        printf("Connected client fd=%d\n", clientFd);
    }
}

void TcpServer::generalRead(const SOCKET fd)
{
    char readbuf[256];

    int nread = sockets::read(fd, readbuf, sizeof(readbuf) - 1);

    if (nread <= 0)
    {
        printf("Disconnected client fd=%d, nick=%s\n", fd, m_fdChannelMap[fd]->getNick().c_str());
        freeClient(m_fdChannelMap[fd]);
    }
    else
    {
        Channel* clientChannel = m_fdChannelMap[fd];
        readbuf[nread] = 0;

        if (readbuf[0] == '/')
        {
            /* Remove any trailing newline. */
            char* p;
            p = strchr(readbuf, '\r'); if (p) *p = 0;
            p = strchr(readbuf, '\n'); if (p) *p = 0;
            /* Check for an argument of the command, after
             * the space. */
            char* arg = strchr(readbuf, ' ');
            if (arg) {
                *arg = 0; /* Terminate command name. */
                arg++; /* Argument is 1 byte after the space. */
            }

            if (!strcmp(readbuf, "/nick") && arg)
            {
                int nicklen = strlen(arg);
                //c->nick = chatMalloc(nicklen + 1);
                //memcpy(c->nick, arg, nicklen + 1);

                std::string snewNick(arg, nicklen);
                clientChannel->setNick(snewNick);
            }
            else
            {
                /* Unsupported command. Send an error. */
                std::string errmsg = "Unsupported command\n";
                int iret = sockets::write(fd, (void*)(errmsg.c_str()), errmsg.size());
            }
        }
        else
        {
            char msg[256];
#ifdef WIN32
            int msglen = sprintf_s(msg, sizeof(msg), "%s> %s", clientChannel->getNick().c_str(), readbuf);
#else
            int msglen = snprintf(msg, sizeof(msg), "%s> %s", clientChannel->getNick().c_str(), readbuf);
#endif

            /* snprintf() return value may be larger than
             * sizeof(msg) in case there is no room for the
             * whole output. */
            if (msglen >= (int)sizeof(msg))
                msglen = sizeof(msg) - 1;
            printf("%s", msg);

            /* Send it to all the other clients. */
            sendMsgToAllClientsBut(fd, msg, msglen);
        }
    }
}

void TcpServer::freeClient(Channel* clientChannel)
{
    if (clientChannel != NULL)
    {
        sockets::close(clientChannel->fd());
        m_fdChannelMap.erase(clientChannel->fd());

        m_clearFds.insert(clientChannel->fd());

        delete clientChannel;
        clientChannel = NULL;
    }
}

void TcpServer::sendMsgToAllClientsBut(SOCKET excluded, char* s, size_t len)
{
    for (auto it : m_fdChannelMap)
    {
        if (it.second == NULL || it.first == excluded || it.first == m_serverSock)
            continue;

        int iret = sockets::write(it.first, s, len);
        int n = 0;
    }
}
