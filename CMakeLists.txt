cmake_minimum_required(VERSION 3.10)

project(AplicacionHttpEducativa)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(http_server src/server.cpp)
add_executable(http_client src/client.cpp)
