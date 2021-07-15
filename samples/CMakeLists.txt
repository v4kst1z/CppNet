cmake_minimum_required(VERSION 3.8)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/samples/) 

add_executable(echo echo.cpp)
target_link_libraries(echo cppnet)

add_executable(tpool_echo tpool_echo.cpp)
target_link_libraries(tpool_echo cppnet)

add_executable(client client.cpp)
target_link_libraries(client cppnet)

add_executable(udp_server udp_server.cpp)
target_link_libraries(udp_server cppnet)

add_executable(async_dns async_dns.cpp)
target_link_libraries(async_dns cppnet)
