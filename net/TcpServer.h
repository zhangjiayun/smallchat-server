#pragma once
#include <set>
#include <map>
#include <memory>
#include "platform.h"
#include "Sockets.h"
#include "Channel.h"

#define SERVER_PORT 7711

class Client;
class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    void run();

private:

    bool Init();
    void InitSocket();
    void UnInit();

    void acceptRead();
    void generalRead(const SOCKET fd);

    void freeClient(Channel* clientChannel);
    void sendMsgToAllClientsBut(SOCKET excluded, char* s, size_t len);

private:
    bool                            m_bInit;
    SOCKET                          m_serverSock;
    int                             m_numChannelFd;
    SOCKET                          m_maxChannelFd;

    std::set<SOCKET>				m_usefulFds;
    std::set<SOCKET>				m_clearFds;
    std::map<SOCKET, Channel*>		m_fdChannelMap;
};
