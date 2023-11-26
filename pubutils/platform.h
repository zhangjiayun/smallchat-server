#pragma once
#include <iostream>
#include <stdint.h>

#define LOG(x) std::cout << x << std::endl

#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include <assert.h>
#include <sys/select.h>
#include <unistd.h>

#include <stdio.h>

typedef int SOCKET;

#define INVALID_SOCKET -1

#define SOCKET_ERROR -1

#define closesocket(s) close(s)

#endif
