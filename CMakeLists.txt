# CMakeList.txt: CppNet 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("CppNet")

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_CXX_COMPILER "clang++")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_BUILD_TYPE Debug)


find_package (Threads)
include_directories(base)
include_directories(net)

# 将源代码添加到此项目的可执行文件。
add_executable (CppNet 
    "CppNet.cpp" 
    "CppNet.h" 
    "./base/ThreadPoolBase.h" 
    "./base/Common.h" 
    "./base/PriorityQueue.h"
    "./base/Variant.h"
    "./net/Event.h" 
    "./net/Epoller.h" 
    "./net/Epoller.cpp" 
    "./net/TimerManager.h"
    "./net/TimerManager.cpp"
    "./net/Looper.h"
    "./net/Looper.cpp"
        base/Socket.h
        base/Ipv4Addr.h
        net/Acceptor.h
        net/IOBuffer.cpp
        net/IOBuffer.h
        net/TcpConnection.cpp
        net/TcpConnection.h
        net/Server.cpp
        net/Server.h
        )
target_link_libraries (CppNet ${CMAKE_THREAD_LIBS_INIT})

# TODO: 如有需要，请添加测试并安装目标。
