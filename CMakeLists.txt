cmake_minimum_required(VERSION 3.28)
project(test_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_project main.cpp
        tdclient.h
        tdclient.cpp)
add_subdirectory(td)
target_link_libraries(test_project PRIVATE Td::TdStatic)