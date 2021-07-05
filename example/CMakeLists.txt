cmake_minimum_required(VERSION 3.8)

find_package(Threads)
include_directories(../base)
include_directories(../net)

file(GLOB BASE_LIST "../base/*.h")
file(GLOB NET_LIST "../net/*.h" "../net/*.cpp")

add_executable(echo echo.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(echo ${CMAKE_THREAD_LIBS_INIT})
