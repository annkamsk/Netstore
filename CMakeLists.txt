cmake_minimum_required(VERSION 3.13)
project(SIK2)

set(CMAKE_CXX_STANDARD 17)

add_executable(SIK2 netstoreclient.cpp err.c group.h server.h client.h err.h client.cpp netstoreserver.cpp)