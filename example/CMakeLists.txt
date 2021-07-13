cmake_minimum_required(VERSION 3.8)


add_executable(echo echo.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(echo ${CMAKE_THREAD_LIBS_INIT})

add_executable(tpool_echo tpool_echo.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(tpool_echo ${CMAKE_THREAD_LIBS_INIT})

add_executable(client client.cpp ${BASE_LIST} ${NET_LIST})
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})