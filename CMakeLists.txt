# CMakeList.txt: CppNet 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("CppNet")

#IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#  SET(CMAKE_INSTALL_PREFIX "/opt/cppnet" CACHE PATH "" FORCE)
#ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set (CMAKE_CXX_COMPILER "clang++")
set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS_DEBUG "-W -g")
#set (CMAKE_CXX_FLAGS_RELEASE)
set (CMAKE_CXX_STANDARD 11)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(Threads)
include_directories(include)

file(GLOB SRC_LIST "src/*")
file(GLOB INC_LIST "include/*")

macro(target_public_headers TARGET)
  set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${ARGN}")
endmacro()

add_library(cppnet ${SRC_LIST})
target_link_libraries(cppnet ${CMAKE_THREAD_LIBS_INIT})
target_public_headers(cppnet ${INC_LIST})

add_library(cppnet_shared SHARED ${SRC_LIST})
target_link_libraries(cppnet_shared ${CMAKE_THREAD_LIBS_INIT})
target_public_headers(cppnet_shared ${INC_LIST})


INSTALL(TARGETS cppnet 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)

INSTALL(TARGETS cppnet_shared 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)

add_subdirectory(samples)


# TODO: 如有需要，请添加测试并安装目标。
