#pragma once
#include <functional>
#include "platform.h"

class Channel
{
public:

    Channel(SOCKET fd);
    ~Channel();

    typedef std::function<void(const SOCKET fd)> ReadEventCallback;

    void onRead();

    void setReadCallback(const ReadEventCallback& cb);

    SOCKET fd() { return m_fd; }

    std::string getNick() { return m_strNick; }
    void setNick(const std::string& sNick) { m_strNick = sNick; }

private:
    ReadEventCallback       m_readCallback;
    SOCKET                  m_fd;
    std::string             m_strNick;
};
