#include "Channel.h"

Channel::Channel(SOCKET fd)
    : m_fd(fd),
    m_readCallback(nullptr)
{
}

Channel::~Channel()
{
}

void Channel::onRead()
{
    if (m_readCallback)
        m_readCallback(m_fd);
}

void Channel::setReadCallback(const ReadEventCallback& cb)
{
    m_readCallback = cb;
}
