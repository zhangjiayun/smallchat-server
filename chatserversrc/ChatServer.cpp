#include "ChatServer.h"

void ChatServer::Init()
{
    m_server.reset(new TcpServer());
}

void ChatServer::start()
{
    if (m_server != nullptr)
        m_server->run();
}
