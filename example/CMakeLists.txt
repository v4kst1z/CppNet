cmake_minimum_required(VERSION 3.8)


add_executable(echo echo.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(echo ${CMAKE_THREAD_LIBS_INIT})

add_executable(tpool_echo tpool_echo.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(tpool_echo ${CMAKE_THREAD_LIBS_INIT})

add_executable(client client.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})

add_executable(udp_server udp_server.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(udp_server ${CMAKE_THREAD_LIBS_INIT})

add_executable(async_dns async_dns.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(async_dns ${CMAKE_THREAD_LIBS_INIT})