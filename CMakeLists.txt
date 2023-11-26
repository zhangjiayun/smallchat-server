cmake_minimum_required (VERSION 2.8)

project (smallchat-server)

set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -O0 -g -Wall -ldl")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O2 -g -Wall -ldl")

add_definitions(-std=c++11)
add_subdirectory(chatserversrc)