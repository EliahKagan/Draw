cmake_minimum_required(VERSION 3.0.0)
project(Draw VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(Draw draw.cpp)

# FIXME: set CXX_STANDARD instead of this doing this hackery
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
