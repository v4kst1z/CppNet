# CMakeList.txt: CppNet 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("CppNet")

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(example)

# TODO: 如有需要，请添加测试并安装目标。
