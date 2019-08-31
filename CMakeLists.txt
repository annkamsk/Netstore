cmake_minimum_required(VERSION 3.13)
project(SIK2)

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

add_executable(SIK2-server server.cpp server.h client.h err.h client.cpp netstoreserver.cpp FileSender.h FileSender.cpp)
add_executable(SIK2-client netstoreclient.cpp server.h client.h err.h client.cpp err.cpp Connection.cpp Connection.h Message.cpp Message.h FileSender.h FileSender.cpp)

TARGET_LINK_LIBRARIES(SIK2-server ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(SIK2-client ${Boost_LIBRARIES})
