#pragma once
#include <memory>
#include "TcpServer.h"
class ChatServer
{

public:
    void Init();
    void start();

private:
    std::unique_ptr<TcpServer>      m_server;

};
